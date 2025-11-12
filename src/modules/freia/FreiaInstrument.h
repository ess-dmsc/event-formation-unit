// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
/// Its functions are called from the main processing loop in FreiaBase
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <common/readout/ess/Parser.h>
#include <common/readout/vmm3/Readout.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/EventBuilder2D.h>
#include <common/types/DetectorType.h>
#include <common/geometry/vmm3/VMM3Geometry.h>
#include <freia/geometry/AmorGeometry.h>
#include <freia/geometry/Config.h>
#include <freia/geometry/EstiaGeometry.h>
#include <freia/geometry/FreiaGeometry.h>
#include <freia/geometry/TBLMBGeometry.h>

#include <memory>
#include <vector>

// Forward declarations
class EV44Serializer;
class Event;
struct BaseSettings;
struct Counters;

namespace Freia {

class FreiaInstrument {

private:
  /// \brief ADC 10-bit limit
  constexpr static uint16_t VMM_ADC_10BIT_LIMIT = 1023;
  /// \brief Stuff that 'ties' Freia together
  Counters &counters;
  BaseSettings &Settings;

  /// \brief Instrument configuration (rings, cassettes, FENs)
  Config Conf;

  /// \brief digital geometry instance
  std::unique_ptr<vmm3::VMM3Geometry> Geom;

  /// \brief factory for geometry instances
  std::unique_ptr<vmm3::VMM3Geometry> createGeometry(const DetectorType &detectorType,
                                               const std::string &InstGeometry,
                                               Statistics &Stats) {
    // Convert to uppercase for case-insensitive comparison
    std::string UpperInstGeometry = InstGeometry;
    std::transform(UpperInstGeometry.begin(), UpperInstGeometry.end(),
                   UpperInstGeometry.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    if (detectorType == DetectorType::FREIA) {
      if (UpperInstGeometry == "AMOR")
        return std::make_unique<AmorGeometry>(Stats, Conf);
      if (UpperInstGeometry == "FREIA")
        return std::make_unique<FreiaGeometry>(Stats, Conf);
      throw std::runtime_error("FREIA detector requires InstrumentGeometry "
                               "'AMOR' or 'Freia', got: " +
                               InstGeometry);
    }

    if (detectorType == DetectorType::TBLMB) {
      if (UpperInstGeometry != "AMOR") {
        throw std::runtime_error(
            "TBLMB detector requires InstrumentGeometry 'AMOR', got: " +
            InstGeometry);
      }
      return std::make_unique<TBLMBGeometry>(Stats, Conf);
    }

    if (detectorType == DetectorType::ESTIA) {
      if (detectorType != UpperInstGeometry) {
        throw std::runtime_error(
            "ESTIA detector requires InstrumentGeometry 'ESTIA', got: " +
            InstGeometry);
      }
      return std::make_unique<EstiaGeometry>(Stats, Conf);
    }

    throw std::runtime_error("Unsupported DetectorType: " +
                             detectorType.toString());
  }

  /// \brief serialiser (and producer) for events
  EV44Serializer &Serializer;

  /// \brief parser for the ESS Readout header
  ESSReadout::Parser &ESSHeaderParser;

public:
  /// \brief 'create' the Freia instrument
  /// based on settings the constructor loads both configuration
  /// and calibration data. It then initialises event builders and
  /// histograms
  FreiaInstrument(Counters &counters, BaseSettings &settings,
                  EV44Serializer &serializer,
                  ESSReadout::Parser &essHeaderParser, Statistics &Stats,
                  const DetectorType &detectorType);

  /// \brief handle loading and application of configuration and calibration
  /// files. This step will throw an exception upon errors.
  void loadConfigAndCalib();

  /// \brief process parsed vmm data into clusters
  void processReadouts(void);

  /// \brief process clusters into events
  void generateEvents(std::vector<Event> &Events);

  /// \brief get geometry statistics
  const vmm3::VMM3Geometry::VmmGeometryCounters &getVmmGeometryStats() const {
    return Geom->getVmmCounters();
  }

  const vmm3::VMM3Geometry &getGeometry() const { return *Geom; }

  /// \brief One builder per cassette, resize in constructor when we have
  /// parsed the configuration file and know the number of cassettes
  std::vector<EventBuilder2D> builders; // reinit in ctor

  /// \brief parser for VMM3 readout data
  vmm3::VMM3Parser VMMParser;
};

} // namespace Freia
