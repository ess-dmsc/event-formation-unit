/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multiblade/caen/Readout.h>
#include <fmt/format.h>

namespace Multiblade {

std::string Readout::debug() const {
  return fmt::format("GTime={} ltime={} digitizer={} chan={} adc={}",
      global_time, local_time, digitizer, channel, adc);
}

}