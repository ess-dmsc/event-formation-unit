
/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>
#include <jalousie/Config.h>
#include <common/EV42Serializer.h>

namespace Jalousie {

struct CLISettings {
  std::string ConfigFile;
};

using namespace memory_sequential_consistent; // Lock free fifo

class JalousieBase : public Detector {
public:
  explicit JalousieBase(BaseSettings const &settings, CLISettings const &LocalSettings);
  ~JalousieBase() { delete eth_ringbuf; }
  void inputThread();
  void processingThread();

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
    int64_t RxPackets;
    int64_t RxBytes;
    int64_t RxIdle;
    int64_t FifoPushErrors;
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing Counters
    int64_t ProcessingIdle;
    int64_t FifoSeqErrors;
    int64_t ReadoutCount;
    int64_t BadModuleId;
    int64_t ChopperPulses;
    int64_t MappingErrors;

    int64_t GeometryErrors;
    int64_t TimingErrors;
    int64_t Events;
    int64_t TxBytes;

    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) Counters;

  CLISettings ModuleSettings;
  Config config;
  uint64_t previous_time{0}; /// < for timing error checks

  void convert_and_enqueue_event(const Readout& readout);
  void process_one_queued_event(EV42Serializer& serializer);
  void force_produce_and_update_kafka_stats(EV42Serializer& serializer, Producer& producer);
};

}
