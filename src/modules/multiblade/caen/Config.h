/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class for configuration of multi blade detector settings
/// reading from json file.
//===----------------------------------------------------------------------===//

#pragma once

#include <multiblade/caen/DigitizerMapping.h>
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
  bool isConfigLoaded() const { return IsConfigLoaded; }

  /// \brief getter fcn for private member variable
  std::string getConfigFile() const { return ConfigFile; }

  /// \brief getter fcn for private member variable
  uint32_t getTimeTickNS() const { return TimeTickNS; }

  /// \brief getter fcn for private member variable
  std::vector<struct DigitizerMapping::Digitiser> getDigitisers() const { return Digitisers; }


  InstrumentGeometry getInstrument() const { return Instrument; }

  DetectorType getDetectorType() const { return DetectorType; }

  uint16_t getWires() const { return NWires; }
  uint16_t getStrips() const { return NStrips; }
  uint16_t getCassettes() const { return NCass; }

  bool filter_time_span{true};
  uint32_t filter_time_span_value{125};

  int max_valid_adc{65534};

  /// local readout timestamp resolution
  uint32_t TimeTickNS{16};

  /// Specify the digital geometry
  std::shared_ptr<DigitizerMapping> Mappings;

private:
  /// \brief helper function to load and parse json file
  void loadConfigFile();

  /// Set to true by loadConfigFile() if all is well
  bool IsConfigLoaded{false};

  /// Name of the json configuration file to load
  std::string ConfigFile{""};

  /// Specify the instrument geometry
  InstrumentGeometry Instrument{InstrumentGeometry::Estia};

  ///
  DetectorType DetectorType{DetectorType::MB18};

  /// for now just hold a vector of the digitisers, \todo later
  /// incorporate in the digital geometry
  std::vector<struct DigitizerMapping::Digitiser> Digitisers;

  uint16_t NCass{0};
  uint16_t NWires{0};
  uint16_t NStrips{0};
};

}
