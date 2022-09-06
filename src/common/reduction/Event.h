/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Event.h
/// \brief Event class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Cluster.h>

/// \class Event Event.h
/// \brief A pairing of clusters in two dimensions, aware of its time bounds.
///        Hits can be added, but not removed. Clusters can be merged but not
///        removed. Timestamps are treated as having an uncertainty of 1 when
///        evaluating dimensions, thus including the endpoints.

class Event {
public:
  Cluster ClusterA; ///< cluster in dimension A
  Cluster ClusterB; ///< cluster in dimension B

public:
  /// \brief Event constructor, selecting planes
  /// \param plane1 id of first plane selected for event
  /// \param plane2 id of second plane selected for event
  Event(uint8_t plane1, uint8_t plane2);

  /// \brief Event default constructor, planes default to 0 and 1
  Event() = default;

  /// \returns id of first plane selected for event
  uint8_t PlaneA() const;
  /// \returns id of second plane selected for event
  uint8_t PlaneB() const;

  /// \brief adds hit to event
  ///        Inserts hit into the appropriate plane.
  ///        If plane is not of two selected planes, nothing is done.
  /// \param hit to be added
  void insert(const Hit &e);

  /// \returns total hit count in both constituent clusters
  size_t total_hit_count() const;

  /// \brief merges a cluster into event.
  ///        Merges the cluster into the appropriate plane.
  ///        If plane is not of two selected planes, nothing is done.
  /// \param cluster to be merged
  /// \post if plane is accepted, merged cluster is cleared
  void merge(Cluster &cluster);

  /// \brief clears both dimensions
  void clear();

  /// \returns true if event contains no hits
  bool empty() const;

  /// \returns true if event has both valid planes
  bool both_planes() const;

  /// \returns earliest timestamp, undefined in case of empty event
  uint64_t time_start() const;
  /// \returns latest timestamp, undefined in case of empty event
  uint64_t time_end() const;
  /// \returns time span, 0 in case of empty event
  uint64_t time_span() const;

  /// \brief calculates the overlapping time span of event and cluster
  /// \param other cluster to be compared
  /// \returns overlapping time span inclusive of end points
  uint64_t time_overlap(const Cluster &other) const;

  /// \brief calculates the time gap of event and cluster
  /// \param other cluster to be compared
  /// \returns time gap
  uint64_t time_gap(const Cluster &other) const;

  /// \returns string describing event bounds and weights
  /// \param verbose also print hits
  std::string to_string(const std::string &prepend, bool verbose) const;

  /// \returns visualizes both clusters with "text graphics"
  std::string visualize(const std::string &prepend, uint8_t downsample_time = 0,
                        uint8_t downsample_coords = 0) const;

private:
  uint8_t PlaneA_{0};
  uint8_t PlaneB_{1};
};
