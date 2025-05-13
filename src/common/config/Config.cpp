// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation og Config base class
//===----------------------------------------------------------------------===//

#include <common/config/Config.h>

#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Configurations {

Config::Config()
  : mMask(NONE)
{

}

Config::Config(const std::string &ConfigFile)
  : mRoot(nullptr)
  , mConfigFile(ConfigFile) {
}

} // namespace Configurations
