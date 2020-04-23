/* Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Stats for NMX detector
///
//===----------------------------------------------------------------------===//
#pragma once

#include <cstdint>

namespace Gem {

  struct NMXStats {
    // Input thread
    int64_t RxPackets{0};
    int64_t RxBytes{0};
    int64_t RxIdle{0};
    int64_t FifoPushErrors{0};
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing thread
    int64_t ProcessingIdle {0};
    int64_t FifoSeqErrors{0};

    // Parser stats
    int64_t ParserFrameSeqErrors{0};
    int64_t ParserFrameMissingErrors{0};
    int64_t ParserFramecounterOverflows{0};
    int64_t ParserTimestampLostErrors{0};
    int64_t ParserTimestampSeqErrors{0};
    int64_t ParserTimestampOverflows{0};
    int64_t ParserBadFrames{0};
    int64_t ParserGoodFrames{0};
    int64_t ParserErrorBytes{0};
    int64_t ParserMarkers{0};
    int64_t ParserData{0};
    int64_t ParserReadouts{0};
    int64_t ParserOverThreshold{0};

    // Hit counters in builder
    int64_t HitsBadPlane{0};
    int64_t HitsBadGeometry{0};
    int64_t HitsBadAdc{0};
    int64_t HitsOutsideRegion{0};
    int64_t HitsGood{0};

    // Clustering
    int64_t ClustersTotal{0};
    int64_t ClustersXOnly{0};
    int64_t ClustersYOnly{0};
    int64_t ClustersXAndY{0};

    // Analysis
    int64_t EventsGood{0};
    int64_t EventsGoodHits{0};
    int64_t EventsBad{0};
    int64_t EventsOutsideRegion{0};
    int64_t EventsFilterRejects{0};
    int64_t EventsGeomErrors{0};

    // Producer
    int64_t TxBytes{0};

    // Kafka stats below are common to all detectors
    int64_t KafkaProduceFails{0};
    int64_t KafkaEvErrors{0};
    int64_t KafkaEvOthers{0};
    int64_t KafkaDrErrors{0};
    int64_t KafkaDrNoErrors{0};
  } __attribute__((aligned(64)));

}
