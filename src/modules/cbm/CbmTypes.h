// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM beam monitor types
///
//===----------------------------------------------------------------------===//

#pragma once

#include <concurrentqueue.h>
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

  static constexpr int MAX = CbmTypes::MM;
  static constexpr int MIN = CbmTypes::TTL;

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
      throw std::invalid_argument("Invalid CBM type integer: " +
                                  std::to_string(type_int));
    }
  }

  const char* to_string() const {
    switch (beamMonitorType) {
      case CbmTypes::TTL:
        return "TTL";
      case CbmTypes::N2GEM:
        return "N2GEM";
      case CbmTypes::IBM:
        return "IBM";
      case CbmTypes::GEM:
        return "GEM";
      case CbmTypes::FC:
        return "FC";
      case CbmTypes::MM:
        return "MM";
      default:
        return "Unknown";
    }
  }

private:
  CbmTypes beamMonitorType;

public:
  operator int() const { return static_cast<int>(beamMonitorType); }

  operator CbmTypes() const { return beamMonitorType; }

  operator uint8_t() const { return static_cast<uint8_t>(beamMonitorType); }
};

} // namespace cbm