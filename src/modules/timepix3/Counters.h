// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Timepix application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>
#include <common/kafka/Producer.h>
#include <cstdint>

struct Counters {
  // Processing Counters - accessed in processing thread

  // Temporary timepix characterising counters

  // System counters
  int64_t FifoSeqErrors{0};
  int64_t ProcessingIdle{0};

  // Events
  int64_t Events{0};
  int64_t PixelErrors{0};
  int64_t TofCount{0};
  int64_t TofNegative{0};
  int64_t PrevTofCount{0};
  int64_t ClusterSizeTooSmall{0};
  int64_t ClusterToShort{0};

  int64_t PixelReadouts{0};
  int64_t InvalidPixelReadout{0};
  int64_t MissTDCCounter{0};
  int64_t MissEVRCounter{0};
  int64_t TDC1RisingReadouts{0};
  int64_t TDC1FallingReadouts{0};
  int64_t TDC2RisingReadouts{0};
  int64_t TDC2FallingReadouts{0};
  int64_t ESSGlobalTimeCounter{0};
  int64_t EventTimeForNextPulse{0};
  int64_t NoGlobalTime{0};
  int64_t EVRPairFound{0};
  int64_t TDCPairFound{0};
  int64_t UnknownTDCReadouts{0};
  int64_t EVRReadoutCounter{0};
  int64_t EVRReadoutDropped{0};
  int64_t TDCReadoutCounter{0};
  int64_t TDCReadoutDropped{0};
  int64_t UndefinedReadoutCounter{0};
  // Kafka stats below are common to all detectors

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};

  // Time measurement counters (in milliseconds)
  int64_t PixelFuturesTimeMs{0};
  int64_t EVRProcessingTimeMs{0};
  int64_t TDCProcessingTimeMs{0};
  int64_t Stage1ProcessingTimeMs{0};
  int64_t Stage2ProcessingTimeMs{0};
  int64_t Stage3ProcessingTimeMs{0};
  int64_t Stage4ProcessingTimeMs{0};

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;

} __attribute__((aligned(64)));
