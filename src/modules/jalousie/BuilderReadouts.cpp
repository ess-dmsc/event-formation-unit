/** Copyright (C) 2017 European Spallation Source ERIC */

#include <jalousie/BuilderReadouts.h>
#include <common/TimeString.h>

#include <common/Trace.h>
#include <sstream>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Jalousie {

// GCOVR_EXCL_START
std::string BuilderReadouts::debug() const {
  std::stringstream ss;
  ss << "  ======================================================\n";
  ss << "  ========           Readouts Builder           ========\n";
  ss << "  ======================================================\n";

//  ss << "  Geometry mappings:\n";
//  ss << digital_geometry_.debug("  ") << "\n";

  return ss.str();
}
// GCOVR_EXCL_STOP

void BuilderReadouts::parse(Buffer<uint8_t> buffer) {

  size_t count = std::min(buffer.size / sizeof(Readout),
                          size_t(9000 / sizeof(Readout)));

  parsed_data.resize(count);
  memcpy(parsed_data.data(), buffer.address, count * sizeof(Readout));
}


}
