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
