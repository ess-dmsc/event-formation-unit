/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <fmt/format.h>
#include <multigrid/mesytec/Readout.h>
#include <sstream>

namespace Multigrid {

// GCOVR_EXCL_START
/// do not care about testing debug code
std::string Readout::debug() const {
  std::stringstream ss;
  ss << fmt::format("trigger={}  time={}", trigger_count, total_time);
  if (external_trigger) {
    ss << " external_trigger";
  } else {
    ss << fmt::format(" bus={} chan={} adc={}", static_cast<uint16_t>(bus),
                      channel, adc);
  }
  // GCOVR_EXCL_STOP

  return ss.str();
}

} // namespace Multigrid
