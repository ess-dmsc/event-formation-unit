// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
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

  // CBM Readouts
  struct cbm::ParserStats CbmStats {
    0
  };

  int64_t DataHeaders{0};

  // Readout processing
  int64_t Event0DReadoutsProcessed{0};
  int64_t IBMReadoutsProcessed{0};
  int64_t TypeNotConfigured{0};

  // Events
  int64_t IBMEvents{0};
  int64_t Event0DEvents{0};

  // Logical and Digital geometry incl. Calibration
  int64_t RingCfgError{0};
  int64_t CbmCounts{0};
  int64_t NPOSCount{0};

  // Configuration errors
  int64_t NoSerializerCfgError{0};

  // Processing time counters
  int64_t ProcessingIdle{0};
  int64_t TimeError{0};

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};
  int64_t TxRawReadoutPackets{0};

} __attribute__((aligned(64)));

} // namespace cbm