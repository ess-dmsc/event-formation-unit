// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/EV42Serializer.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>
#include <dream/Counters.h>

namespace Jalousie {

struct DreamSettings {
  //std::string ConfigFile;
  //
  //
  //
};

using namespace memory_sequential_consistent; // Lock free fifo

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings, struct DreamSettings &LocalDreamSettings);
  ~DreamBase() = default;

  void inputThread();
  void processingThread();






  /// \todo figure out the right size  of EthernetBufferMaxEntries
  static const int EthernetBufferMaxEntries {2000};
  static const int EthernetBufferSize {9000}; /// bytes
  static const int KafkaBufferSize {124000}; /// entries ~ 1MB

  // Ideally should match the CPU speed, but as this varies across
  // CPU versions we just select something in the 'middle'. This is
  // used to get an approximate time for periodic housekeeping so
  // it is not critical that this is precise.
  const int TSC_MHZ = 2900;

protected:
  /// Shared between input_thread and processing_thread
  CircularFifo<unsigned int, EthernetBufferMaxEntries> InputFifo;
  /// \todo the number 11 is a workaround
  RingBuffer<EthernetBufferSize> RxRingbuffer{EthernetBufferMaxEntries + 11};

  struct Counters Counters;
  DreamSettings DreamModuleSettings;
  EV42Serializer * Serializer;
};

}
