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
#include <multigrid/mgmesytec/MesytecReadout.h>
#include <vector>

namespace Multigrid {

class VMMR16Parser {
public:
  void spoof_high_time(bool spoof);
  bool spoof_high_time() const;

  /// \brief parse n 32 bit words from mesytec VMMR-8/16 card
  /// returns number of discarded(uparsed) bytes
  size_t parse(Buffer<uint32_t> buffer);

  size_t trigger_count() const;

  uint64_t time() const;
  bool externalTrigger() const;

  std::vector<MesytecReadout> converted_data;

private:

  MesytecReadout hit;

  size_t trigger_count_{0};
  uint32_t high_time_{0};

  bool external_trigger_{false};

  bool spoof_high_time_{false};
  uint32_t previous_low_time_{0};

  uint32_t previous_high_time_{0};

};

}