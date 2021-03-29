// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief SoNDe detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/Detector.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

struct SoNDeSettings {
  std::string fileprefix{""};
};



/** ----------------------------------------------------- */

class SONDEIDEABase : public Detector {
public:
  explicit SONDEIDEABase(BaseSettings const & settings, SoNDeSettings & localSoNDeSettings);
  ~SONDEIDEABase() = default;

  void input_thread();
  void processing_thread();

protected:

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t rx_idle;
    int64_t fifo_push_errors;
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing and Output counters
    int64_t processing_idle;
    int64_t rx_pkt_triggertime;
    int64_t rx_pkt_singleevent;
    int64_t rx_pkt_multievent;
    int64_t rx_pkt_unsupported;
    int64_t rx_events;
    int64_t rx_geometry_errors;
    int64_t tx_bytes;
    int64_t rx_seq_errors;
    int64_t fifo_synch_errors;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) mystats;

  struct SoNDeSettings SoNDeSettings;
};
