/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file Event.h
/// \brief Event class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Cluster.h>

/// \class Event Event.h
/// \brief A pairing of clusters in two dimensions, aware of its time bounds.
///        Hits can be added, but not removed. Clusters can be merged but not
///        removed. Timestamps are treated as having an uncertainty of 1 when
///        evaluating dimensions, thus including the endpoints.

class Event {
private:
  uint8_t plane1_ {0};
  uint8_t plane2_ {1};

public:
  Cluster c1; ///< cluster in dimension 1
  Cluster c2; ///< cluster in dimension 2

  /// \brief Event default constructor, planes default to 0 and 1
  Event() = default;

  /// \brief Event constructor, selecting planes
  /// \param plane1 id of first plane selected for event
  /// \param plane2 id of second plane selected for event
  Event(uint8_t plane1, uint8_t plane2);

  /// \returns id of first plane selected for event
  uint8_t plane1() const;
  /// \returns id of second plane selected for event
  uint8_t plane2() const;

  /// \brief adds hit to event
  ///        Inserts hit into the appropriate plane.
  ///        If plane is not of two selected planes, nothing is done.
  /// \param hit to be added
  void insert(const Hit &e);

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

  /// \brief calculates the overlapping time span of event and cluster
  /// allowing for a (small) time gap
  /// \param other cluster to be compared
  /// \param timegap allowed time gap
  /// \returns overlapping time span inclusive of end points
  uint64_t time_overlap(const Cluster &other, uint64_t timegap) const;

  /// \returns string describing event bounds and weights
  /// \param verbose also print hits
  std::string debug(bool verbose = false) const;
};
