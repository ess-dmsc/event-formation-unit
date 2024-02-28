// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>
#include <cbm/geometry/Parser.h>

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;

  // VMM3a Readouts
  struct TTLMonitor::ParserStats TTLMonStats;

  // Logical and Digital geometry incl. Calibration
  int64_t RingCfgErrors;
  int64_t FENCfgErrors;
  int64_t ChannelCfgErrors;
  int64_t TOFErrors;
  int64_t MonitorCounts;
  int64_t MonitorIgnored;
  int64_t MaxADC;

  //
  int64_t ProcessingIdle;
  int64_t TimeErrors;
  struct ESSReadout::ESSTime::Stats_t TimeStats;
  int64_t TxBytes;

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;

} __attribute__((aligned(64)));
