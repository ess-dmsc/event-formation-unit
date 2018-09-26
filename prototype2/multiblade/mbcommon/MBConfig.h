/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class for configuration of multi blade detector settings
/// reading from json file.
//===----------------------------------------------------------------------===//

#pragma once

#include <mbcaen/MB16Detector.h>
#include <string>


class MBConfig {
public:

  ///
  MBConfig(){};

  /// \brief get configuration from file
  explicit MBConfig(std::string jsonfile);

private:
  enum class InstrumentGeometry {Estia, Freia};

  /// \brief helper function to load and parse json file
  void loadConfigFile();

  /// Name of the json configuratio file to load
  std::string ConfigFile;

  /// Specify the instrument geometry
  InstrumentGeometry instrument{InstrumentGeometry::Estia};

  /// Specify the digital geometry
  MB16Detector * detector{nullptr};

  /// local readout timestamp resolution
  uint32_t TimeTickNS{16};

  /// Set to true by loadConfigFile() if all is well
  bool IsValidConfig{false};
};
