// Copyright (C) 2020 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/Producer.h>
#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/matching/GapMatcher.h>

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors;

  // ESSReadout parser
  int64_t ErrorESSHeaders;
  // int64_t RingRx[24];

  // VMM3a Readouts
  struct ESSReadout::VMM3ParserStats VMMStats;

  // Logical and Digital geometry incl. Calibration
  int64_t RingMappingErrors;
  int64_t FENMappingErrors;
  int64_t HybridMappingErrors;
  int64_t InvalidXCoord;
  int64_t InvalidYCoord;
  int64_t MaxTOFErrors;
  int64_t MaxADC;
  int64_t ADCBelowThreshold;

  //
  int64_t ProcessingIdle;
  int64_t Events;
  int64_t EventsNoCoincidence;
  int64_t EventsMatchedClusters;
  int64_t EventsMatchedWireOnly;
  int64_t EventsMatchedStripOnly;
  int64_t EventsInvalidStripGap;
  int64_t EventsInvalidWireGap;
  struct GapMatcherStats MatcherStats;

  int64_t PixelErrors;
  int64_t TimeErrors;
  int64_t TxRawReadoutPackets;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout;

} __attribute__((aligned(64)));
