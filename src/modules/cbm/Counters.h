// Copyright (C) 2022-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/Producer.h>
#include <cstdint>
#include <modules/cbm/geometry/Parser.h>

namespace cbm {

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors{0};

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats {
    0
  };
  int64_t ErrorESSHeaders{0};

  // CBM Readouts
  struct cbm::ParserStats CbmStats {
    0
  };

  // Readout processing
  int64_t TTLReadoutsProcessed{0};
  int64_t IBMReadoutsProcessed{0};
  int64_t TypeNotSupported{0};

  // Events
  int64_t IBMEvents{0};
  int64_t TTLEvents{0};

  // Logical and Digital geometry incl. Calibration
  int64_t RingCfgError{0};
  int64_t CbmCounts{0};

  // Configuration errors
  int64_t NoSerializerCfgError{0};

  // Processing time counters
  int64_t ProcessingIdle{0};
  int64_t TimeError{0};

  struct ESSReadout::ESSReferenceTime::Stats_t TimeStats;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;
  KafkaEventHandler KafkaEventStats;
  DeliveryReportHandler KafkaMsgDeliveryStats;

} __attribute__((aligned(64)));

} // namespace cbm