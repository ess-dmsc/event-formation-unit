// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/Cuboid.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/geometry/DreamMantle.h>
#include <dream/geometry/SUMO.h>
#include <set>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

DreamGeometry::DreamGeometry(Statistics &Stats, const Config &Config)
    : Geometry(Stats, Config) {
  // Scan configuration to determine which module types are used
  std::set<Config::ModuleType> requiredTypes;

  for (int ring = 0; ring <= DreamConfig.MaxRing; ++ring) {
    for (int fen = 0; fen <= DreamConfig.MaxFEN; ++fen) {
      const Config::ModuleParms &Parms = DreamConfig.RMConfig[ring][fen];
      if (Parms.Initialised) {
        requiredTypes.insert(Parms.Type);
      }
    }
  }

  // Create and register only the modules that are actually used
  for (const auto &Type : requiredTypes) {
    switch (Type) {
    case Config::BwEndCap:
      SubModules[Config::BwEndCap] =
          std::make_unique<SUMO>(Stats, BwecCassettes, BwecStrips, "bwec");
      XTRACE(INIT, INF, "Initialized Backward Endcap (SUMO)");
      break;

    case Config::FwEndCap:
      SubModules[Config::FwEndCap] =
          std::make_unique<SUMO>(Stats, FwecCassettes, FwecStrips, "fwec");
      XTRACE(INIT, INF, "Initialized Forward Endcap (SUMO)");
      break;

    case Config::DreamMantle:
      SubModules[Config::DreamMantle] =
          std::make_unique<DreamMantle>(Stats, MantleStrips);
      XTRACE(INIT, INF, "Initialized Dream Mantle");
      break;

    case Config::HR:
      SubModules[Config::HR] = std::make_unique<Cuboid>(Stats, "hr");
      XTRACE(INIT, INF, "Initialized Cuboid (HR)");
      break;

    case Config::SANS:
      SubModules[Config::SANS] = std::make_unique<Cuboid>(Stats, "sans");
      XTRACE(INIT, INF, "Initialized Cuboid (SANS)");
      break;

    default:
      XTRACE(INIT, WAR, "Unknown module type: %d", Type);
      break;
    }
  }

  XTRACE(INIT, INF, "Initialized %zu geometry module(s)", SubModules.size());
}

uint32_t DreamGeometry::calcPixelImpl(const DataParser::CDTReadout &Data) const {
  int Ring = Data.FiberId / 2;
  const Config::ModuleParms &Parms = getModuleParms(Ring, Data.FENId);

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  // Direct array access by enum index for O(1) lookup
  if (SubModules[Parms.Type]) {
    Pixel = SubModules[Parms.Type]->calcPixelId(Parms, Data);
  } else {
    XTRACE(DATA, WAR, "No geometry module registered for type: %d", Parms.Type);
    return 0;
  }

  if (Pixel == 0) {
    XTRACE(DATA, WAR, "Invalid pixel returned in module: %i", Parms.Type);
    return 0;
  }

  int Offset = getPixelOffset(Parms.Type);
  if (Offset == -1) {
    XTRACE(DATA, WAR, "No offset given for module %i", Parms.Type);
    return 0;
  }
  int GlobalPixel = Offset + Pixel;
  XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
  return GlobalPixel;
}

int DreamGeometry::getPixelOffset(Config::ModuleType Type) const {
  int RetVal{-1};
  switch (Type) {
  case Config::FwEndCap:
    RetVal = 0;
    break;
  case Config::BwEndCap:
    RetVal = 71680; ///< Offset value from ICD
    break;
  case Config::DreamMantle:
    RetVal = 229376; ///< Offset value from ICD
    break;
  case Config::SANS:
    RetVal = 720896; ///< Offset value from ICD
    break;
  case Config::HR:
    RetVal = 1122304; ///< Offset value from ICD
    break;
  default:
    XTRACE(DATA, WAR, "Module type not valid for DREAM");
    break;
  }
  return RetVal;
}

} // namespace Dream
