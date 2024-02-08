// Copyright (C) 2020-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief pixel generator
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/Producer.h>

namespace PerfGen {

class PerfGenBase : public Detector {
public:
  PerfGenBase(BaseSettings const &settings);
  ~PerfGenBase() {}

  void processingThread();

  static const int kafka_buffer_size = 124000; /// entries

protected:
  struct {
    // Processing Counters
    int64_t events_udder;
    int64_t tx_bytes;
    // Kafka stats below are common to all detectors
    struct Producer::ProducerStats KafkaStats;
  } __attribute__((aligned(64))) mystats;
};

} // namespace PerfGen
