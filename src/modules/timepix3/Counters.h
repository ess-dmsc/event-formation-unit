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
#include <cstdint>

struct Counters {
  // Processing Counters - accessed in processing thread

  // Temporary timepix characterising counters
  int64_t PixelReadoutFromBeforeTDC;

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
  int64_t TDC1RisingReadouts;
  int64_t TDC1FallingReadouts;
  int64_t TDC2RisingReadouts;
  int64_t TDC2FallingReadouts;
  int64_t MissTDCPair;
  int64_t MissEVRPair;
  int64_t FoundEVRandTDCPairs;
  int64_t UnknownTDCReadouts;
  int64_t GlobalTimestampReadouts;
  int64_t EVRTimestampReadouts;
  int64_t UndefinedReadouts;
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails;
  int64_t kafka_ev_errors;
  int64_t kafka_ev_others;
  int64_t kafka_dr_errors;
  int64_t kafka_dr_noerrors;
} __attribute__((aligned(64)));
