// Copyright (C) 2018-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/matching/GapMatcher.h>
// #include <cmath>
// #include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void GapMatcher::setMinimumTimeGap(uint64_t minimum_time_gap) {
  minimum_time_gap_ = minimum_time_gap;
}

void GapMatcher::setSplitMultiEvents(bool split_multi_events,
                                     float coefficient_low,
                                     float coefficient_high) {
  split_multi_events_ = split_multi_events;
  coefficient_low_ = coefficient_low;
  coefficient_high_ = coefficient_high;
}

void GapMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.timeStart() < c2.timeStart();
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

    if (!evt.empty() && (evt.timeGap(*cluster) > minimum_time_gap_)) {
      XTRACE(CLUSTER, DEB, "time gap too large, gap is %u, max is %u", evt.timeGap(*cluster), minimum_time_gap_);
      checkAndStashEvent(evt);
      evt.clear();
    }

    evt.merge(*cluster);

    unmatched_clusters_.pop_front();
  }

  /// If anything remains
  if (!evt.empty()) {
    if (flush) {
      checkAndStashEvent(evt);
    } else {
      requeue_clusters(evt);
    }
  }
}

std::string GapMatcher::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << AbstractMatcher::config(prepend);
  ss << prepend << fmt::format("minimum_time_gap: {}\n", minimum_time_gap_);
  return ss.str();
}

void GapMatcher::splitAndStashEvent(Event evt) {
  Cluster new_cluster_a_1;
  Cluster new_cluster_a_2;
  Cluster new_cluster_b_1;
  Cluster new_cluster_b_2;
  splitCluster(evt.ClusterA, &new_cluster_a_1, &new_cluster_a_2);
  splitCluster(evt.ClusterB, &new_cluster_b_1, &new_cluster_b_2);
  Event new_event_1;
  Event new_event_2;
  bool a1_b1_match = clustersMatch(new_cluster_a_1, new_cluster_b_1);
  bool a2_b2_match = clustersMatch(new_cluster_a_2, new_cluster_b_2);
  bool a1_b2_match = clustersMatch(new_cluster_a_1, new_cluster_b_2);
  bool a2_b1_match = clustersMatch(new_cluster_a_2, new_cluster_b_1);
  XTRACE(CLUSTER, DEB, "a1xb1: %d, a2xb2: %d, a1xb2: %d, a2xb1: %d",
         a1_b1_match, a2_b2_match, a1_b2_match, a2_b1_match);

  if (a1_b1_match && a2_b2_match && !a1_b2_match && !a2_b1_match) {
    new_event_1.merge(new_cluster_a_1);
    new_event_1.merge(new_cluster_b_1);
    new_event_2.merge(new_cluster_a_2);
    new_event_2.merge(new_cluster_b_2);
  } else if (a1_b2_match && a2_b1_match && !a1_b1_match && !a2_b2_match) {
    new_event_1.merge(new_cluster_a_1);
    new_event_1.merge(new_cluster_b_2);
    new_event_2.merge(new_cluster_a_2);
    new_event_2.merge(new_cluster_b_1);
  } else {
    XTRACE(CLUSTER, DEB,
           "Unable to match clusters into two distinct events, discarding "
           "readouts");
    Stats.DiscardedSpanTooLarge++;

    //\todo count discarded multievents
    return;
  }
  Stats.SplitSpanTooLarge++;
  stashEvent(new_event_1);
  stashEvent(new_event_2);
  evt.clear();
}

void GapMatcher::splitCluster(Cluster cluster, Cluster *new_cluster_1,
                              Cluster *new_cluster_2) {
  sortByIncreasingCoordinate(cluster.hits);
  uint last_coord = 0;
  bool filled_cluster_1 = false;

  for (Hit hit : cluster.hits) {
    // no hits added to any clusters yet, automatically added to cluster 1
    if ((new_cluster_1->empty())) {
      last_coord = hit.coordinate;
      new_cluster_1->insert(hit);
      XTRACE(CLUSTER, DEB, "Adding first hit to cluster 1");
    }
    // cluster 1 not marked as full yet, and gap is small enough to still be
    // cluster 1
    else if ((hit.coordinate - last_coord <= minimum_coord_gap_) &&
             (!filled_cluster_1)) {
      last_coord = hit.coordinate;
      new_cluster_1->insert(hit);
      XTRACE(CLUSTER, DEB, "Adding another hit to cluster 1");
    }
    // cluster 1 not marked as full yet, and large enough gap to start new
    // cluster hit goes into cluster 2
    else if ((hit.coordinate - last_coord > minimum_coord_gap_) &&
             (!filled_cluster_1)) {
      last_coord = hit.coordinate;
      filled_cluster_1 = true;
      new_cluster_2->insert(hit);
      XTRACE(CLUSTER, DEB, "Adding first hit to cluster 2");
    }
    // cluster 1 marked as full, and gap small enough to keep adding to cluster
    // 2
    else if ((hit.coordinate - last_coord <= minimum_coord_gap_) &&
             (filled_cluster_1)) {
      last_coord = hit.coordinate;
      new_cluster_2->insert(hit);
      XTRACE(CLUSTER, DEB, "Adding another hit to cluster 2");
    }
    // cluster 1 full, and gap large enough to finish cluster 2 and start a 3rd
    // 3 or more clusters not supported, clusters are cleared and method exited
    else if ((hit.coordinate - last_coord > minimum_coord_gap_) &&
             (filled_cluster_1)) {
      XTRACE(CLUSTER, DEB,
             "More than 2 distinct clusters in plane, clearing clusters and "
             "returning");
      new_cluster_1->clear();
      new_cluster_2->clear();
      return;
    }
  }
}

bool GapMatcher::clustersMatch(Cluster cluster_a, Cluster cluster_b) {
  if ((cluster_a.weightSum() * coefficient_low_ <= cluster_b.weightSum()) &&
      (cluster_a.weightSum() * coefficient_high_ >= cluster_b.weightSum())) {
    return true;
  } else {
    return false;
  }
}

void GapMatcher::checkAndStashEvent(Event evt) {
  if (!split_multi_events_) {
    XTRACE(CLUSTER, DEB, "Stashing event");
    stashEvent(evt);
    evt.clear();
    return;
  }
  if ((evt.ClusterA.coordSpan() < maximum_coord_span_) and
      (evt.ClusterB.coordSpan() < maximum_coord_span_)) {
    XTRACE(CLUSTER, DEB, "Stashing event, span isn't too large");
    XTRACE(CLUSTER, DEB, "Cluster A coord span = %u, Cluster B coord span = %u",
           evt.ClusterA.coordSpan(), evt.ClusterB.coordSpan());
    stashEvent(evt);
    evt.clear();
  } else { // split clusters by coord gaps and attempt to match based on ADC
           // values
    XTRACE(CLUSTER, DEB, "Span is too large, attempting to split event");
    XTRACE(CLUSTER, DEB,
           "Cluster A spans %u and contains %u hits, and Cluster B spans "
           "%u and contains %u hits",
           evt.ClusterA.coordSpan(), evt.ClusterA.hitCount(),
           evt.ClusterB.coordSpan(), evt.ClusterB.hitCount());
    Stats.SpanTooLarge++;
    splitAndStashEvent(evt);
    evt.clear();
  }
}
