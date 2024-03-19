// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM beam monitor types
///
//===----------------------------------------------------------------------===//

#pragma once

#include <stdexcept>
#include <string>
#include <sys/types.h>

namespace cbm {

// Implemented according to beam monitor ICD
// https://project.esss.dk/nextcloud/index.php/f/17959238

class CbmType {

public:
  enum CbmTypes {
    TTL = 0x01,
    N2GEM = 0x02,
    IBM = 0x03,
    GEM = 0x04,
    FC = 0x05,
    MM = 0x06
  };

  CbmType(const std::string &typeStr) {
    if (typeStr == "TTL") {
      beamMonitorType = CbmTypes::TTL;
    } else if (typeStr == "N2GEM") {
      beamMonitorType = CbmTypes::N2GEM;
    } else if (typeStr == "IBM") {
      beamMonitorType = CbmTypes::IBM;
    } else if (typeStr == "GEM") {
      beamMonitorType = CbmTypes::GEM;
    } else if (typeStr == "FC") {
      beamMonitorType = CbmTypes::FC;
    } else if (typeStr == "MM") {
      beamMonitorType = CbmTypes::MM;
    } else {
      throw std::invalid_argument("Invalid CBM type string: " + typeStr);
    }
  }

  CbmType(const int type_int) {
    if (type_int == 0x01) {
      beamMonitorType = CbmTypes::TTL;
    } else if (type_int == 0x02) {
      beamMonitorType = CbmTypes::N2GEM;
    } else if (type_int == 0x03) {
      beamMonitorType = CbmTypes::IBM;
    } else if (type_int == 0x04) {
      beamMonitorType = CbmTypes::GEM;
    } else if (type_int == 0x05) {
      beamMonitorType = CbmTypes::FC;
    } else if (type_int == 0x06) {
      beamMonitorType = CbmTypes::MM;
    } else {
      throw std::invalid_argument("Invalid CBM type integer: " + std::to_string(type_int));
    }
  }

private:
  CbmTypes beamMonitorType;

public:
    operator int() const {
        return static_cast<int>(beamMonitorType);
    }

    operator CbmTypes() const {
        return beamMonitorType;
    }

    operator uint8_t() const {
        return static_cast<uint8_t>(beamMonitorType);
    }
};

} // namespace cbm