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

class Cluster {
protected:
  std::vector<Hit> hits;

public:
  /// \brief adds hit to event's plane
  /// \param hit to be added, need not be in any particular order
  void insert_hit(const Hit &hit);

  bool empty() const;
  bool valid() const;
  int16_t plane() const;
  size_t hit_count() const;

  /// calculated as hits are added
  uint16_t coord_start() const;
  uint16_t coord_end() const;
  uint16_t coord_span() const;

  uint64_t time_start() const;
  uint64_t time_end() const;
  uint64_t time_span() const;

  double weight_sum() const;

  double coord_mass() const;
  double coord_center() const;

  double time_mass() const;
  double time_center() const;

  void merge(Cluster &other);
  double time_overlap(const Cluster &other) const;
  bool time_touch(const Cluster &other) const;

  /// \brief prints values for debug purposes
  std::string debug() const;

private:
  int16_t plane_{-1};

  uint16_t coord_start_{0};
  uint16_t coord_end_{0};

  uint64_t time_start_{0};
  uint64_t time_end_{0};

  double weight_sum_{0.0};
  double coord_mass_{0.0};   /// sum of coord*weight
  double time_mass_{0.0};   /// sum of time*weight

};
