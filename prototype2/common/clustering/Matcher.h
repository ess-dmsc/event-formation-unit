/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractMatcher.h>

class Matcher : public AbstractMatcher {
public:
  explicit Matcher(uint64_t maxDeltaTime);

  void insert(uint8_t plane, ClusterContainer &c) override;
  void match(bool flush) override;

  ClusterContainer unmatched_clusters;
private:
  uint64_t pMaxDeltaTime{0};

  bool ready_to_be_matched(double time) const;

  uint64_t delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;

  uint64_t latest_x{0};
  uint64_t latest_y{0};
};
