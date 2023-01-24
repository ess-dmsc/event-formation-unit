// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief pixel generator (delete?)
/// \todo delete?
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>

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
    int64_t kafka_produce_calls;
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) mystats;
};

} // namespace PerfGen
