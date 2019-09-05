/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
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
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

struct SoNDeSettings {
  std::string fileprefix{""};
};

using namespace memory_sequential_consistent; // Lock free fifo

static constexpr int TscMHz{2900}; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class SONDEIDEABase : public Detector {
public:
  explicit SONDEIDEABase(BaseSettings const & settings, SoNDeSettings & localSoNDeSettings);
  ~SONDEIDEABase() = default;

  void input_thread();
  void processing_thread();

  /** \todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 124000; /**< events */

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    int64_t pad[5]; // cppcheck-suppress unusedStructMember

    // Processing and Output counters
    int64_t rx_idle1;
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
