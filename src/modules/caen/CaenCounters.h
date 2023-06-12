// Copyright (C) 2019-2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CAEN application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>
#include <modules/caen/geometry/CDCalibration.h>

namespace Caen {

struct CaenCounters {
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

  // Logical and Digital geometry incl. Calibration
  /// \todo replace by struct, atm this causes problems.
  int64_t RingErrors;
  int64_t FENErrors;
  int64_t TubeErrors;
  int64_t AmplitudeZero;
  int64_t OutsideTube;
  struct CDCalibration::Stats Calibration;

  // Events
  int64_t Events;
  int64_t PixelErrors;
  int64_t EventsUdder;

  // Time
  struct ESSReadout::ESSTime::Stats_t TimeStats;

  int64_t TxBytes;
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails;
  int64_t kafka_ev_errors;
  int64_t kafka_ev_others;
  int64_t kafka_dr_errors;
  int64_t kafka_dr_noerrors;
} __attribute__((aligned(64)));
};
