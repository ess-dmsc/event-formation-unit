/* Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file MultiHitMatcher2D.h
/// \brief MultiHitMatcher2D class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/matching/MultiHitMatcher2D.h>
// #include <cmath>
// #include <algorithm>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

void MultiHitMatcher2D::set_minimum_time_gap(uint64_t minimum_time_gap) {
  minimum_time_gap_ = minimum_time_gap;
}

void MultiHitMatcher2D::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_start() < c2.time_start();
  });

  XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u",
         unmatched_clusters_.size());

  Event evt{PlaneA, PlaneB};
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster)) {
      XTRACE(CLUSTER, DEB, "not ready to be matched");
      break;
    }

    if (!evt.empty() && (evt.time_gap(*cluster) > minimum_time_gap_)) {
      XTRACE(CLUSTER, DEB, "time gap too large");
      if ((evt.ClusterA.coord_span() < maximum_coord_span_) and (evt.ClusterB.coord_span() < maximum_coord_span_)){
        XTRACE(CLUSTER, DEB, "Stashing event, span isn't too large");
        stash_event(evt);
        evt.clear();
      }
      else{ //split clusters by coord gaps and attempt to match based on ADC values
        XTRACE(CLUSTER, DEB, "Span is too large, attempting to split event");
        split_and_stash_event(evt);
      }
    }

    evt.merge(*cluster);

    unmatched_clusters_.pop_front();
  }

  /// If anything remains
  if (!evt.empty()) {
    if (flush) {
      stash_event(evt);
    } else {
      requeue_clusters(evt);
    }
  }
}

std::string MultiHitMatcher2D::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << AbstractMatcher::config(prepend);
  ss << prepend << fmt::format("minimum_time_gap: {}\n", minimum_time_gap_);
  return ss.str();
}

void MultiHitMatcher2D::split_and_stash_event(Event evt){
  Cluster new_cluster;
  std::vector<Cluster> new_clusters_a = split_cluster(evt.ClusterA);
  std::vector<Cluster> new_clusters_b = split_cluster(evt.ClusterB);
  std::vector<Event> new_events;

  for (Cluster cluster_a : new_clusters_a){
    bool matched_a_to_b = false;
    for (Cluster cluster_b : new_clusters_b){
      if (clusters_match(cluster_a, cluster_b)){
        if (matched_a_to_b){
          XTRACE(CLUSTER, DEB, "More than 1 Cluster in B plane matched Cluster in A plane, can't split events by ADC values, discarding them");
          return;
        }
        XTRACE(CLUSTER, DEB, "Matched cluster a and b");
        Event new_evt;
        new_evt.merge(cluster_a);
        new_evt.merge(cluster_b);
        new_events.push_back(new_evt);
      }
    }
  }
  for (Cluster cluster_b : new_clusters_b){
    bool matched_b_to_a = false;
    for (Cluster cluster_a : new_clusters_a){
      if (clusters_match(cluster_a, cluster_b)){
        if (matched_b_to_a){
          XTRACE(DATA, DEB, "More than 1 Cluster in A plane matched Cluster in B plane, can't split events by ADC values, discarding them");
          return;
        }
        matched_b_to_a = true;
      }
    }
  }

  for (Event evt : new_events){
    stash_event(evt);
  }

}

std::vector<Cluster> MultiHitMatcher2D::split_cluster(Cluster cluster){
  Cluster new_cluster;
  std::vector<Cluster> new_clusters;
  sort_by_increasing_coordinate(cluster.hits);
  uint last_coord = 0;

  for(Hit hit : cluster.hits){
    if ((!new_cluster.empty()) && (hit.coordinate - last_coord > minimum_coord_gap_)){
      new_clusters.push_back(new_cluster);
      new_cluster.clear();
      last_coord = hit.coordinate;
      new_cluster.insert(hit);
    }
    else{
      last_coord = hit.coordinate;
      new_cluster.insert(hit);
    }
  }
  if (!new_cluster.empty()){
    new_clusters.push_back(new_cluster);
    new_cluster.clear();
  }

  for(Cluster cluster : new_clusters){
    XTRACE(DATA, DEB, "New cluster created, starts at coord %u, ends at coord %u", cluster.coord_start(), cluster.coord_end());
  }

  return new_clusters;
}

bool MultiHitMatcher2D::clusters_match(Cluster cluster_a, Cluster cluster_b){
  if ((cluster_a.weight_sum() * coefficient_ >= cluster_b.weight_sum() - allowance_) && (cluster_a.weight_sum() * coefficient_ <= cluster_b.weight_sum() + allowance_)){
    return true;
  }
  else{
    return false;
  }
}