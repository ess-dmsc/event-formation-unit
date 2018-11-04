/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractMatcher.h>

class OverlapMatcher : public AbstractMatcher {
public:
  OverlapMatcher(uint64_t latency)
      : AbstractMatcher(latency) {}
  OverlapMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
      : AbstractMatcher(latency, plane1, plane2) {}

  void match(bool flush) override;
};
