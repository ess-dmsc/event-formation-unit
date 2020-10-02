// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie Readout data utilities
///
//===----------------------------------------------------------------------===//

#include <jalousie/Readout.h>
#include <fmt/format.h>

namespace Jalousie {

std::string Readout::debug() const {
  return fmt::format(" board={}/{} time={} anode={} cathode={}",
      board, sub_id, time, anode, cathode);
}

}
