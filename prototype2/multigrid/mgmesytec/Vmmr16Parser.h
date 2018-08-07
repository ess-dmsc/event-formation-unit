/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/Buffer.h>
#include <multigrid/mgmesytec/Hit.h>
#include <vector>

struct MgStats {
  size_t readouts{0}; /**< number of channels read out */
  size_t discards{0}; /**< readouts discarded due to adc thresholds */
  size_t triggers{0}; /**< number of 0x58 blocks in packet */
  size_t events{0};   /**< number of events from this packets */
  size_t tx_bytes{0}; /**< number of bytes produced by librdkafka */
  size_t geometry_errors{0}; /**< number of invalid pixels from readout */
  size_t badtriggers{0}; /**< number of empty triggers or triggers without valid data */
};

class VMMR16Parser {
public:
  void spoof_high_time(bool spoof);
  bool spoof_high_time() const;

  /** \brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void parse(const Buffer& buffer, MgStats& stats);

  uint64_t time() const;
  bool externalTrigger() const;

  std::vector<MGHit> converted_data;

private:

  MGHit hit;

  size_t trigger_count_{0};
  uint32_t high_time_ {0};

  bool external_trigger_ {false};

  bool spoof_high_time_ {false};
  uint32_t previous_low_time_{0};

  // clang-format off
// Mesytec Datasheet: VMMR-8/16 v00.01
  enum Type : uint32_t {
    Header            = 0x40000000,
    ExtendedTimeStamp = 0x20000000,
    DataEvent1        = 0x30000000,
    DataEvent2        = 0x10000000,
    EndOfEvent        = 0xc0000000,
    FillDummy         = 0x00000000
  };
// clang-format on

};
