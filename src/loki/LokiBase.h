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
#include <common/EV42Serializer.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>

namespace Loki {

struct LokiSettings {
  uint32_t Unused;
};

using namespace memory_sequential_consistent; // Lock free fifo

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &Settings, struct LokiSettings &LocalLokiSettings);
  ~LokiBase() = default;
  void inputThread();
  void processingThread();
  void testImageUdder(EV42Serializer& FlatBuffer);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int EthernetBufferMaxEntries = 500;
  static const int EthernetBufferSize = 9000; /// bytes
  static const int KafkaBufferSize = 124000; /// entries ~ 1MB
  // Ideally should match the CPU speed, bust at this varies across
  // CPU versions we just select something in the 'middle'. This is
  // used to get an approximate time for periodic housekeeping so
  // it is not critical that this is precise.
  const int TSC_MHZ = 2900;

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, EthernetBufferMaxEntries> InputFifo;
  /// \todo the number 11 is a workaround
  RingBuffer<EthernetBufferSize> EthernetRingbuffer{EthernetBufferMaxEntries + 11};

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
