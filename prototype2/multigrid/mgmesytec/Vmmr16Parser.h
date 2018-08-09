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

namespace Multigrid {

class VMMR16Parser {
public:
  void spoof_high_time(bool spoof);
  bool spoof_high_time() const;

  /** \brief parse n 32 bit words from mesytec VMMR-8/16 card */
  size_t parse(Buffer<uint32_t> buffer);

  size_t trigger_count() const;

  uint64_t time() const;
  bool externalTrigger() const;

  std::vector<Hit> converted_data;

private:

  Hit hit;

  size_t trigger_count_{0};
  uint32_t high_time_{0};

  bool external_trigger_{false};

  bool spoof_high_time_{false};
  uint32_t previous_low_time_{0};

  uint32_t previous_high_time_{0};

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

}