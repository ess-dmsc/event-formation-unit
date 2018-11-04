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

  void match(bool flush) override;

private:
  uint64_t max_delta_time_{0};

  uint64_t delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;
};
