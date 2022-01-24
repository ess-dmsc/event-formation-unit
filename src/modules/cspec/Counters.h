// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/VMM3Parser.h>

struct Counters {
    // Input Counters - accessed in input thread
    int64_t RxPackets;
    int64_t RxBytes;
    int64_t RxIdle;
    int64_t FifoPushErrors;
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing Counters - accessed in processing thread
    int64_t FifoSeqErrors;


    // ESSReadout parser
    struct ESSReadout::ESSHeaderStats ReadoutStats;
    int64_t ErrorESSHeaders;
    //int64_t RingRx[24];

    // VMM3a Readouts
    struct ESSReadout::VMM3ParserStats VMMStats;

    // Logical and Digital geometry incl. Calibration
    int64_t HybridErrors;
    int64_t TOFErrors;
    int64_t MaxADC;
    int64_t MappingErrors;

    //
    int64_t ProcessingIdle;
    int64_t Events;
    int64_t EventsNoCoincidence;
    int64_t EventsMatchedWireOnly;
    int64_t EventsMatchedGridOnly;
    int64_t EventsTooLargeGridSpan;
    int64_t EventsMatchedClusters;
    int64_t PixelErrors;
    int64_t TimeErrors;
    int64_t RingErrors;
    int64_t FENErrors;
    struct ESSReadout::ESSTime::Stats_t TimeStats;
    int64_t TxBytes;
    // Kafka stats below are common to all detectors
    struct Producer::ProducerStats KafkaStats;

  } __attribute__((aligned(64)));
