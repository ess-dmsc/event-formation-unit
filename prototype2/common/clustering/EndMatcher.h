/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractMatcher.h>

class EndMatcher : public AbstractMatcher {
public:
  EndMatcher(uint64_t max_delta_time, uint64_t latency);
  EndMatcher(uint64_t max_delta_time, uint64_t latency,
             uint8_t plane1, uint8_t plane2);

  void insert(uint8_t plane, ClusterContainer &c) override;
  void match(bool flush) override;

private:
  uint64_t max_delta_time_{0};
  uint64_t latency_{0};
  uint8_t plane1_{0};
  uint8_t plane2_{1};

  ClusterContainer unmatched_clusters_;
  uint64_t latest_x_{0};
  uint64_t latest_y_{0};

  bool ready_to_be_matched(double time) const;

  uint64_t delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;

};
