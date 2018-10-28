/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multiblade/caen/Readout.h>
#include <fmt/format.h>

namespace Multiblade {

std::string Readout::debug() const {
  return fmt::format("GTime={} digitizer={} ltime={} chan={} adc={}",
      global_time, digitizer, local_time, channel, adc);
}

}