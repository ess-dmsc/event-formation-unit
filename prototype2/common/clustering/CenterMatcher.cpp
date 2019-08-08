/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file CenterMatcher.h
/// \brief CenterMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/CenterMatcher.h>
#include <common/Trace.h>
// #include <cmath>
// #include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

CenterMatcher::CenterMatcher(uint64_t latency, uint64_t time_gap, std::string time_algorithm)
   : AbstractMatcher(latency), allowed_time_gap_(time_gap),time_algorithm_(time_algorithm)  {}

// CenterMatcher::CenterMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
//     : AbstractMatcher(latency, plane1, plane2) {}

  void CenterMatcher::match(bool flush) {
    
    if(time_algorithm_ == "center-of-mass") {
        unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
            return c1.time_center() < c2.time_center();
        });
    }
    else if(time_algorithm_ == "charge2") {
        unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
            return c1.time_center2() < c2.time_center2();
        });
    }  
    else if(time_algorithm_ == "utpc") {
        unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
            return c1.time_utpc(false) < c2.time_utpc(false);
        });
    }    
    else if(time_algorithm_ == "utpc_weighted") {
        unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
            return c1.time_utpc(true) < c2.time_utpc(true);
        });
    }
      

    XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u", unmatched_clusters_.size());

    Event evt{plane1_, plane2_};
    Event pulse_evt{pulse_plane_, pulse_plane_};
    
    while (!unmatched_clusters_.empty()) {

      auto cluster = unmatched_clusters_.begin();

      if (!flush && !ready_to_be_matched(*cluster)) {
        XTRACE(CLUSTER, DEB, "not ready to be matched");
        break;
      }

      //if the event is complete in both planes, stash it
      if (evt.both_planes())
      {
        XTRACE(CLUSTER, DEB, "stash complete plane1/2 event");
        stash_event(evt);
        evt.clear();
      }
      if (cluster->plane() == pulse_plane_) {
        pulse_evt.merge(*cluster);
        stash_event(pulse_evt);
        pulse_evt.clear();
      }
      else 
      {
        if(!evt.empty())
        {
          if (evt.time_gap(*cluster) > allowed_time_gap_) {
            XTRACE(CLUSTER, DEB, "time gap too large");
            stash_event(evt);
            evt.clear();
          }
          
          if (cluster->plane() == 1)
          {
            if(!evt.empty1())
            {
              XTRACE(CLUSTER, DEB, "stash plane 1 event");
              stash_event(evt);
              evt.clear();
            }
          }
          else 
          {
            if(!evt.empty2())
            {
              XTRACE(CLUSTER, DEB, "stash plane 2 event");
              stash_event(evt);
              evt.clear();
            }
          }
        }
        //Add only to the cluster, if the plane is empty
        evt.merge(*cluster);
      }
      unmatched_clusters_.pop_front();
    }

  if (!evt.empty()) {
    if (flush) {
      // If flushing, stash it
      stash_event(evt);
    } else {
      // Else return to queue
      // \todo this needs explicit testing
      if (!evt.c1.empty())
        unmatched_clusters_.push_front(evt.c1);
      if (!evt.c2.empty())
        unmatched_clusters_.push_front(evt.c2);
    }
  }
}
