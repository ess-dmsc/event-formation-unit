/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Readout.h>
#include <fmt/format.h>

namespace Jalousie {

std::string Readout::debug() const {
  return fmt::format(" board={}/{} time={} anode={} cathode={}",
      board, sub_id, time, anode, cathode);
}

}