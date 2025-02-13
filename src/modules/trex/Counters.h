// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/VMM3Parser.h>

#include <cinttypes>

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors{0};

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders{0};
  // int64_t RingRx[24];

  // VMM3a Readouts
  struct ESSReadout::VMM3ParserStats VMMStats;

  // Logical and Digital geometry incl. Calibration
  int64_t HybridMappingErrors{0};
  int64_t TOFErrors{0};
  int64_t MinADC{0};
  int64_t MaxADC{0};
  int64_t MappingErrors{0};

  //
  int64_t ProcessingIdle{0};
  int64_t Events{0};
  int64_t ClustersNoCoincidence{0};
  int64_t ClustersMatchedWireOnly{0};
  int64_t ClustersMatchedGridOnly{0};
  int64_t ClustersTooLargeGridSpan{0};
  int64_t EventsMatchedClusters{0};
  int64_t PixelErrors{0};
  int64_t TimeErrors;
  int64_t TxRawReadoutPackets{0};
  struct ESSReadout::ESSReferenceTime::Stats_t TimeStats;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};
} __attribute__((aligned(64)));
