/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class for configuration of multi blade detector settings
/// reading from json file.
//===----------------------------------------------------------------------===//

#pragma once

#include <caen/DigitizerMapping.h>
#include <memory>
#include <string>
#include <vector>

namespace Multiblade {

class Config {
public:
  enum class InstrumentGeometry { Estia, Freia };
  enum class DetectorType { MB16, MB18 };

  ///
  Config() = default;
  ~Config() = default;

  /// \brief get configuration from file
  explicit Config(std::string jsonfile);

  /// \brief getter fcn for private member variable
  bool isConfigLoaded() { return IsConfigLoaded; }

  /// \brief getter fcn for private member variable
  std::string getConfigFile() { return ConfigFile; }

  // /// \brief getter fcn for private member variable
  std::shared_ptr<DigitizerMapping> getDigitizers() { return Detector; }

  /// \brief getter fcn for private member variable
  uint32_t getTimeTickNS() { return TimeTickNS; }

  /// \brief getter fcn for private member variable
  auto getDigitisers() { return Digitisers; }

  InstrumentGeometry getInstrument() { return Instrument; }

  DetectorType getDetectorType() { return DetectorType; }

private:
  /// \brief helper function to load and parse json file
  void loadConfigFile();

  /// Set to true by loadConfigFile() if all is well
  bool IsConfigLoaded{false};

  /// Name of the json configuration file to load
  std::string ConfigFile{""};

  /// Specify the digital geometry
  std::shared_ptr<DigitizerMapping> Detector;

  /// Specify the instrument geometry
  InstrumentGeometry Instrument{InstrumentGeometry::Estia};

  ///
  DetectorType DetectorType{DetectorType::MB18};

  /// local readout timestamp resolution
  uint32_t TimeTickNS{16};

  /// for now just hold a vector of the digitisers, \todo later
  /// incorporate in the digital geometry
  std::vector<struct DigitizerMapping::Digitiser> Digitisers;

  uint16_t NCass{0};
  uint16_t NWires{0};
  uint16_t NStrips{0};
};

}
