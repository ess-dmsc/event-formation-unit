/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>

namespace Loki {

struct LokiSettings {
  uint32_t unused;
};

using namespace memory_sequential_consistent; // Lock free fifo

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &settings, struct LokiSettings &LocalLokiSettings);
  ~LokiBase() { delete EthernetRingbuffer; }
  void input_thread();
  void processing_thread();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int EthernetBufferMaxEntries = 500;
  static const int EthernetBufferSize = 9000; /// bytes
  static const int KafkaBufferSize = 124000; /// entries ~ 1MB

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, EthernetBufferMaxEntries> InputFifo;
  RingBuffer<EthernetBufferSize> *EthernetRingbuffer;

  struct {
    // Input Counters - accessed in input thread
    int64_t RxPackets;
    int64_t RxBytes;
    int64_t FifoPushErrors;
    int64_t PaddingFor64ByteAlignment[5]; // cppcheck-suppress unusedStructMember

    // Processing Counters - accessed in processing thread
    int64_t FifoSeqErrors;
    int64_t ReadoutsErrorBytes;
    int64_t ReadoutsCount;

    int64_t RxIdle;
    int64_t Events;
    int64_t EventsUdder;
    int64_t GeometryErrors;
    int64_t TxBytes;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) Counters;

  LokiSettings LokiModuleSettings;
};

}
