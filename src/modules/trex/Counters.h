// Copyright (C) 2022 European Spallation Source, see LICENSE file
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
  int64_t FifoSeqErrors;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;
  // int64_t RingRx[24];

  // VMM3a Readouts
  struct ESSReadout::VMM3ParserStats VMMStats;

  // Logical and Digital geometry incl. Calibration
  int64_t HybridMappingErrors;
  int64_t TOFErrors;
  int64_t MinADC;
  int64_t MaxADC;
  int64_t MappingErrors;

  //
  int64_t ProcessingIdle;
  int64_t Events;
  int64_t ClustersNoCoincidence;
  int64_t ClustersMatchedWireOnly;
  int64_t ClustersMatchedGridOnly;
  int64_t ClustersTooLargeGridSpan;
  int64_t EventsMatchedClusters;
  int64_t PixelErrors;
  int64_t TimeErrors;
  struct ESSReadout::ESSReferenceTime::Stats_t TimeStats;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout;

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;

} __attribute__((aligned(64)));
