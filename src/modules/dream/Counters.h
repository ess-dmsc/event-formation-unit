// Copyright (C) 2021 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/kafka/Producer.h>
#include <common/readout/ess/Parser.h>

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;

  // DREAM DataParser
  int64_t DataHeaders{0};
  int64_t Readouts{0};
  int64_t BufferErrors{0};
  int64_t DataLenErrors{0};
  int64_t FiberErrors{0};
  int64_t FENErrors{0};

  int64_t RingMappingErrors{0};
  int64_t FENMappingErrors{0};
  int64_t ConfigErrors{0};

  //
  int64_t ProcessingIdle;
  int64_t Events;
  int64_t GeometryErrors;

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};

  int64_t TxRawReadoutPackets{0};

  // Kafka stats below are common to all detectors
  struct Producer::ProducerStats KafkaStats;
} __attribute__((aligned(64)));
