/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/Readout.h>
#include <sstream>
#include <fmt/format.h>

namespace Multigrid {

std::string Readout::debug() const {
  std::stringstream ss;
  ss << fmt::format("trigger={}  time={}", trigger_count, total_time);
  if (external_trigger) {
    ss << " external_trigger";
  } else {
    ss << fmt::format(" bus={} chan={} adc={}",
                      static_cast<uint16_t>(bus), channel, adc);
  }

/// these should no longer be of interest
//  ss << " high_time=" << high_time;
//  ss << " low_time=" << low_time;
//  ss << " time_diff=" << time_diff;

  // \todo maybe use this at some point when readout makes use of it in some way...
  //ss << " module=" << static_cast<uint16_t>(module);

  return ss.str();
}

}