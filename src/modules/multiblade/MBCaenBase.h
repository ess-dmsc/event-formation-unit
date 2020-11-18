// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
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
#include <multiblade/Counters.h>
#include <multiblade/caen/Readout.h>

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
  ~CAENBase() { delete EthernetRingbuffer; }
  void input_thread();
  void processing_thread();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int EthernetBufferMaxEntries = 500;
  static const int EthernetBufferSize = 9000; /// bytes
  static const int KafkaBufferSize{124000}; /// entries ~ 1MB

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, EthernetBufferMaxEntries> InputFifo;
  RingBuffer<EthernetBufferSize> *EthernetRingbuffer;
  struct Counters Counters;
  CAENSettings MBCAENSettings;
};

}
