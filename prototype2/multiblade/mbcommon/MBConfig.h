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
#include <memory>
#include <string>
#include <vector>

namespace Multiblade {

class MBConfig {
public:
  enum class InstrumentGeometry { Estia, Freia };

  ///
  MBConfig() = default;
  ~MBConfig() = default;

  /// \brief get configuration from file
  explicit MBConfig(std::string jsonfile);

  /// \brief getter fcn for private member variable
  bool isConfigLoaded() { return IsConfigLoaded; }

  /// \brief getter fcn for private member variable
  std::string getConfigFile() { return ConfigFile; }

  /// \brief getter fcn for private member variable
  std::shared_ptr<MB16Detector> getDetector() { return Detector; }

  /// \brief getter fcn for private member variable
  uint32_t getTimeTickNS() { return TimeTickNS; }

  /// \brief getter fcn for private member variable
  auto getDigitisers() { return Digitisers; }

  InstrumentGeometry getInstrument() { return Instrument; }

private:
  /// \brief helper function to load and parse json file
  void loadConfigFile();

  /// Set to true by loadConfigFile() if all is well
  bool IsConfigLoaded{false};

  /// Name of the json configuration file to load
  std::string ConfigFile{""};

  /// Specify the digital geometry
  std::shared_ptr<MB16Detector> Detector;

  /// Specify the instrument geometry
  InstrumentGeometry Instrument{InstrumentGeometry::Estia};

  /// local readout timestamp resolution
  uint32_t TimeTickNS{16};

  /// for now just hold a vector of the digitisers, \todo later
  /// incorporate in the digital geometry
  std::vector<struct MB16Detector::Digitiser> Digitisers;
};

}
