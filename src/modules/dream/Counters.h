// Copyright (C) 2021 - 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>

struct Counters {
  // Processing Counters - accessed in processing thread
  int64_t FifoSeqErrors;

  // ESSReadout parser
  struct ESSReadout::ESSHeaderStats ReadoutStats;
  int64_t ErrorESSHeaders;

  // DREAM DataParser
  int64_t Readouts;
  int64_t DataHeaders;
  int64_t ErrorDataHeaders;
  int64_t ErrorBytes;
  int64_t RingErrors;
  int64_t FENErrors;
  int64_t ConfigErrors;

  //
  int64_t ProcessingIdle;
  int64_t Events;
  int64_t EventsUdder;
  int64_t MappingErrors;
  int64_t GeometryErrors;
  int64_t TxBytes;
  // Kafka stats below are common to all detectors
  int64_t kafka_produce_fails;
  int64_t kafka_ev_errors;
  int64_t kafka_ev_others;
  int64_t kafka_dr_errors;
  int64_t kafka_dr_noerrors;
} __attribute__((aligned(64)));
