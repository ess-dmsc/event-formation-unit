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
#include <caen/Config.h>
#include <multiblade/caen/Readout.h>

namespace Multiblade {

struct CAENSettings {
  std::string FilePrefix{""};
  std::string ConfigFile{""};
  uint32_t H5SplitTime{0}; // split files every N seconds (0 is inactive)
};



class CAENBase : public Detector {
public:
  CAENBase(BaseSettings const &settings, struct CAENSettings &LocalMBCAENSettings);
  ~CAENBase() = default;

  void input_thread();
  void processing_thread();

protected:

  struct {
    // Input Counters - accessed in input thread
    int64_t RxPackets;
    int64_t RxBytes;
    int64_t RxIdle;
    int64_t FifoPushErrors;
    int64_t PaddingFor64ByteAlignment[4]; // cppcheck-suppress unusedStructMember

    // Processing Counters - accessed in processing thread
    int64_t FifoSeqErrors;
    int64_t ReadoutsErrorVersion;
    int64_t ReadoutsSeqErrors;
    int64_t ReadoutsErrorBytes;
    int64_t ReadoutsCount;
    int64_t ReadoutsGood;
    int64_t ReadoutsMonitor; /// \todo so far hardcoded
    int64_t ReadoutsInvalidAdc;
    int64_t ReadoutsInvalidChannel;
    int64_t ReadoutsInvalidPlane;
    int64_t FiltersMaxTimeSpan;
    int64_t FiltersMaxMulti1;
    int64_t FiltersMaxMulti2;
    int64_t ProcessingIdle;
    int64_t Events;
    int64_t EventsUdder;
    int64_t EventsNoCoincidence;
    int64_t EventsNotAdjacent;
    int64_t GeometryErrors;
    int64_t TxBytes;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) Counters;

  CAENSettings MBCAENSettings;
  Config MultibladeConfig;
};

}
