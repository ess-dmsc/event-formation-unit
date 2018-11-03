/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Cluster.h>
#include <limits>
#include <list>
#include <string>

class Event {
public:
  Cluster x, y; /// tracks in x and y planes

  /// \brief adds hit to event
  /// \param hit to be added
  void insert_hit(const Hit &e);

  void merge(Cluster &cluster);

  bool empty() const;

  uint64_t time_end() const;
  uint64_t time_start() const;
  uint64_t time_span() const;
  uint64_t time_overlap(const Cluster &other) const;
  bool time_overlap_thresh(const Cluster &other, double thresh) const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};
