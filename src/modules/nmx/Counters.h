// Copyright (C) 2022 - 2026 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/matching/GapMatcher.h>

#include <cinttypes>

struct Counters {
  // ESSReadout parser
  int64_t ErrorESSHeaders;

  // VMM3a Readouts
  struct vmm3::VMM3ParserStats VMMStats;

  // Logical and Digital geometry incl. Calibration
  int64_t HybridMappingErrors;
  /// \todo TREX still using this, we should decouple it from NMX Counters
  int64_t MinADC;
  int64_t MappingErrors;

  struct GapMatcherStats MatcherStats;
  //
  int64_t ProcessingIdle;
  int64_t Events;
  int64_t ClustersNoCoincidence;
  int64_t ClustersMatchedXOnly;
  int64_t ClustersMatchedYOnly;
  int64_t ClustersTooLargeSpanX;
  int64_t ClustersTooLargeSpanY;
  int64_t ClustersTooSmallSpanX;
  int64_t ClustersTooSmallSpanY;
  int64_t ClustersTooLargeTimeSpan;
  int64_t EventsMatchedClusters;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout;
} __attribute__((aligned(64)));
