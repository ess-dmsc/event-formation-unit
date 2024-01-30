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
  int64_t PixelReadoutFromBeforeTDC{0};
  // System counters
  int64_t FifoSeqErrors{0};
  int64_t ProcessingIdle{0};

  // Events
  int64_t Events{0};
  int64_t PixelErrors{0};
  int64_t TofCount{0};
  int64_t TofNegative{0};
  int64_t PrevTofCount{0};
  int64_t PrevTofNegative{0};
  int64_t TofHigh{0};
  int64_t PrevTofHigh{0};

  int64_t TxBytes{0};
  int64_t PixelReadouts{0};
  int64_t MissTDCCounter{0};
  int64_t MissEVRCounter{0};
  int64_t TDC1RisingReadouts{0};
  int64_t TDC1FallingReadouts{0};
  int64_t TDC2RisingReadouts{0};
  int64_t TDC2FallingReadouts{0};
  int64_t EventTimeForNextPulse{0};
  int64_t NoGlobalTime{0};
  int64_t EVRPairFound{0};
  int64_t TDCPairFound{0};
  int64_t UnknownTDCReadouts{0};
  int64_t GlobalTimestampReadouts{0};
  int64_t EVRTimeStampReadouts{0};
  int64_t TDCTimeStampReadout{0};
  int64_t UndefinedReadouts{0};
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails{0};
  int64_t kafka_ev_errors{0};
  int64_t kafka_ev_others{0};
  int64_t kafka_dr_errors{0};
  int64_t kafka_dr_noerrors{0};

} __attribute__((aligned(64)));
