// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/CDTGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

int CDTGeometry::getPixel(
          Config::ModuleParms & Parms,
          DataParser::DreamReadout & Data) {

    int Pixel{0};
    XTRACE(DATA, DEB, "Type: %u", Parms.Type);

    switch (Parms.Type) {
      case Config::BwEndCap: /* fallthrough */
      case Config::FwEndCap: {
        XTRACE(DATA, DEB, "Bw or Fw EndCap");
        /// \todo fix and check all values
        uint8_t Sumo = 6; // function of Cathode/Anode
        uint8_t Cassette = 0; // function of Cathode/Anode?
        uint8_t Counter = 0; // function of Cathode/Anode?
        uint8_t Wire = Data.Cathode; // maybe
        uint8_t Strip = Data.Anode; // maybe
        XTRACE(DATA, DEB, "Index %u, Sumo %u, Cassette %u, Counter %u, Wire %u, Strip %u",
            Parms.P1.Sector, Sumo, Cassette, Counter, Wire, Strip);
        Pixel = sumo.getPixel(Parms.P1.Sector, Sumo, Cassette, Counter, Wire, Strip);
        }
        break;

      case Config::Mantle: {
        XTRACE(DATA, DEB, "Mantle");
        /// \todo fix and check all values
        uint8_t Counter = 0; /// \todo part of anode field?
        uint8_t Wire = Data.Cathode;
        uint8_t Strip = Data.Anode;
        Pixel = mantle.getPixelId(Parms.P1.MU, Parms.P2.Cassette, Counter, Wire, Strip);
        }
        break;

      case Config::HR: /* fallthrough */
      case Config::SANS: {
        XTRACE(DATA, DEB, "HR or SANS");
        /// \todo fix and check all values
        uint8_t Cassette = 0; // function of Cathode/Anode?
        uint8_t Counter = 0; // function of Cathode/Anode?
        uint8_t Wire = Data.Cathode; // maybe
        uint8_t Strip = Data.Anode; // maybe
        uint8_t Rotate = 0; // for now
        Pixel = cuboid.getPixelId(Parms.P1.Index, Cassette, Counter, Wire, Strip, Rotate);
        }
        break;
      default:
        XTRACE(DATA, WAR, "Unknown detector");
        break;
    }
    int GlobalPixel = getPixelOffset(Parms.Type) + Pixel;
    XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
    return GlobalPixel;
}

int CDTGeometry::getPixelOffset(Config::ModuleType Type) {
  int RetVal{-1};
  switch (Type) {
    case Config::FwEndCap:
      RetVal = 0;
      break;
    case Config::BwEndCap:
      RetVal = 71680;
      break;
    case Config::Mantle:
      RetVal = 229376;
      break;
    case Config::SANS:
      RetVal = 720896;
      break;
    case Config::HR:
      RetVal = 1122304;
      break;
  }
  return RetVal;;
}

} // namespace
