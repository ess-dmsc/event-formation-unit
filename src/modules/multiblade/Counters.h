// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multiblade application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>

struct Counters {
    // Input Counters - accessed in input thread
    int64_t RxPackets;
    int64_t RxBytes;
    int64_t RxIdle;
    int64_t FifoPushErrors;
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing Counters - accessed in processing thread
    int64_t FifoSeqErrors;
    int64_t ReadoutsErrorVersion;
    int64_t ReadoutsSeqErrors;
    int64_t ReadoutsErrorBytes;
    int64_t PacketBadDigitizer;
    int64_t ReadoutsCount;
    int64_t ReadoutsGood;
    int64_t ReadoutsMonitor; /// \todo so far hardcoded
    int64_t ReadoutsInvalidAdc;
    int64_t ReadoutsInvalidChannel;
    int64_t ReadoutsInvalidPlane;
    int64_t ReadoutsTimerWraps;
    int64_t ProcessingIdle;
    int64_t Events;
    int64_t EventsUdder;
    int64_t EventsNoCoincidence;
    int64_t EventsMatchedClusters;
    int64_t EventsInvalidStripGap;
    int64_t EventsInvalidWireGap;
    int64_t GeometryErrors;
    int64_t TxBytes;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64)));
