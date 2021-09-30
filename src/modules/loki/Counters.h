// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <readout/common/Parser.h>

struct Counters {
  // Input Counters - accessed in input thread
  int64_t RxPackets;
  int64_t RxBytes;
  int64_t FifoPushErrors;
  int64_t RxIdle;
  int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

  // Processing Counters - accessed in processing thread

  // System counters
  int64_t FifoSeqErrors;
  int64_t ProcessingIdle;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;

  // LoKI DataParser
  int64_t DataHeaders;
  int64_t Readouts;
  int64_t ReadoutsBadAmpl;
  int64_t ErrorDataHeaders;
  int64_t ErrorBytes;

  // Logical and Digital geometry incl. Calibration
  int64_t RingErrors;
  int64_t FENErrors;
  int64_t CalibrationErrors;
  int64_t ReadoutsClampLow;
  int64_t ReadoutsClampHigh;

  // Events
  int64_t Events;
  int64_t PixelErrors;
  int64_t OutsideRegion;
  int64_t EventsUdder;
  int64_t TofCount;
  int64_t TofNegative;
  int64_t PrevTofCount;
  int64_t PrevTofNegative;

  int64_t TxBytes;
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails;
  int64_t kafka_ev_errors;
  int64_t kafka_ev_others;
  int64_t kafka_dr_errors;
  int64_t kafka_dr_noerrors;
} __attribute__((aligned(64)));
