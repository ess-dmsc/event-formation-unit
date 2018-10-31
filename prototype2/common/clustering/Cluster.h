/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cluster: container of hits, aware of its bounds and weight
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Hit.h>

struct Cluster {

  int16_t plane {-1};

  /// \brief adds hit to event's plane
  /// \param hit to be added
  void insert_hit(const Hit &hit);

  std::vector<Hit> entries;
  bool empty() const;

  /// calculated as hits are added
  uint16_t coord_start{0};
  uint16_t coord_end{0};
  uint16_t coord_span() const;

  uint64_t time_start{0};
  uint64_t time_end{0};
  uint64_t time_span() const;

  double weight_sum{0.0};

  double coord_mass{0.0};   /// sum of strip*weight
  double coord_center() const;

  double time_mass{0.0};   /// sum of time*weight
  double time_center() const;


  void merge(Cluster& other);
  double time_overlap(const Cluster& other) const;
  bool time_touch(const Cluster& other) const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};
