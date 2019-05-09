
/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief MBCAEN detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>
#include <caen/Config.h>
#include <prototype2/multiblade/caen/Readout.h>

namespace Multiblade {

struct CAENSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  uint32_t H5SplitTime{0}; // split files every N seconds (0 is inactive)
};

using namespace memory_sequential_consistent; // Lock free fifo

class CAENBase : public Detector {
public:
  CAENBase(BaseSettings const &settings, struct CAENSettings &LocalMBCAENSettings);
  ~CAENBase() { delete eth_ringbuf; }
  void input_thread();
  void processing_thread();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 500;
  static const int eth_buffer_size = 9000; /// bytes

  static const int kafka_buffer_size = 124000; /// entries

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5]; // cppcheck-suppress unusedStructMember

    // Processing Counters
    int64_t events_udder;
    int64_t readouts_monitor; // \todo so far hardcoded
    int64_t readouts_invalid_adc;
    int64_t readouts_invalid_ch;
    int64_t readouts_invalid_plane;
    int64_t filters_max_time_span;
    int64_t filters_max_multi1;
    int64_t filters_max_multi2;
    int64_t rx_idle1;
    int64_t rx_readouts;
    int64_t readouts_ok;
    int64_t readouts_error_bytes;
    int64_t readouts_seq_errors;
    int64_t tx_bytes;
    int64_t events;
    int64_t events_no_coincidence;
    int64_t events_not_adjacent;
    int64_t geometry_errors;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) mystats;

  struct CAENSettings MBCAENSettings;
  Config mb_opts;
};

}
