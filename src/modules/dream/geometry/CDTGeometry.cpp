// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <dream/geometry/CDTGeometry.h>
#include <dream/geometry/Config.h>

namespace Dream {

int CDTGeometry::getPixelOffset(Config::ModuleType Type) {
  switch (Type) {
    case Config::FwEndCap:
      return 0;
      break;
    case Config::BwEndCap:
      return 71680;
      break;
    case Config::Mantle:
      return 229376;
      break;
    case Config::SANS:
      return 720896;
      break;
    case Config::HR:
      return 1122304;
      break;
    default:
      return -1; // unreachable
      break;
    __builtin_unreachable();
    return 0;
  }
}

}
