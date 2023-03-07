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

struct Counters {
  // Processing Counters - accessed in processing thread

  // System counters
  int64_t FifoSeqErrors;
  int64_t ProcessingIdle;

  // Events
  int64_t Events;
  int64_t PixelErrors;
  int64_t TofCount;
  int64_t TofNegative;
  int64_t PrevTofCount;
  int64_t PrevTofNegative;
  int64_t TofHigh;
  int64_t PrevTofHigh;

  int64_t TxBytes;
  int64_t PixelReadouts;
  int64_t TDCReadouts;
  int64_t GlobalTimestampReadouts;
  int64_t UndefinedReadouts;
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails;
  int64_t kafka_ev_errors;
  int64_t kafka_ev_others;
  int64_t kafka_dr_errors;
  int64_t kafka_dr_noerrors;
} __attribute__((aligned(64)));
