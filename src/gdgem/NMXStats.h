/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
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
    // Input Counters
    int64_t rx_packets{0};
    int64_t rx_bytes{0};
    int64_t fifo_push_errors{0};
    // \todo Morten: explain. What is going on here?
    int64_t pad_a[5]; // cppcheck-suppress unusedStructMember

    // Processing thread
    int64_t processing_idle {0};
    int64_t fifo_seq_errors{0};

    // Parser stats
    //int64_t fc_seq_errors;
    int64_t parser_frame_seq_errors{0};
    int64_t parser_frame_missing_errors{0};
    int64_t parser_framecounter_overflows{0};
    int64_t parser_timestamp_lost_errors{0};
    int64_t parser_timestamp_seq_errors{0};
    int64_t parser_timestamp_overflows{0};
    int64_t parser_bad_frames{0};
    int64_t parser_good_frames{0};
    int64_t parser_error_bytes{0};
    int64_t parser_markers{0};
    int64_t parser_data{0};
    int64_t parser_readouts{0};
    int64_t parser_over_threshold{0};


    // Hit counters in builder
    int64_t hits_bad_plane{0};
    int64_t hits_bad_geometry{0};
    int64_t hits_bad_adc{0};
    int64_t hits_good{0};


    // Clustering
    int64_t clusters_total{0};
    int64_t clusters_x_only{0};
    int64_t clusters_y_only{0};
    int64_t clusters_xy{0};

    // Analysis
    int64_t events_good{0};
    int64_t events_good_hits{0};
    int64_t events_bad{0};
    int64_t events_filter_rejects{0};
    int64_t events_geom_errors{0};



    // Producer
    int64_t tx_bytes{0};

    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails{0};
    int64_t kafka_ev_errors{0};
    int64_t kafka_ev_others{0};
    int64_t kafka_dr_errors{0};
    int64_t kafka_dr_noerrors{0};
  } __attribute__((aligned(64)));

}
