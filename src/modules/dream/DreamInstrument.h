// Copyright (C) 2021 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and DREAM readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/readout/ess/Parser.h>
#include <common/detector/BaseSettings.h>
#include "common/Statistics.h"
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/kafka/EV44Serializer.h>
#include <common/time/TimeString.h>

#include <cstdint>
#include <dream/Counters.h>
#include <dream/geometry/Config.h>
#include <dream/geometry/Geometry.h>
#include <dream/readout/DataParser.h>
#include <dream/DreamInstrument.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/geometry/Geometry.h>
#include <dream/geometry/HeimdalGeometry.h>
#include <dream/geometry/MagicGeometry.h>

#include <memory>

namespace Dream {

using namespace esstime;
using namespace ESSReadout;

template <int Type_t>
class DreamInstrument {
private:
  struct Counters &Counters;
  BaseSettings &Settings;
  Config DreamConfiguration;
  std::unique_ptr<Geometry> Geom;
  EV44Serializer &Serializer;
  ESSReadout::Parser &ESSHeaderParser;

public:
  /// \brief 'create' the DREAM instrument
  ///
  /// loads configuration and calibration files, calculate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  DreamInstrument(Statistics &Stats, struct Counters &counters,
                  BaseSettings &settings, EV44Serializer &serializer,
                  ESSReadout::Parser &essHeaderParser);

  ~DreamInstrument() = default;

  /// \brief process parsed vmm data into clusters
  void processReadouts();

  /// \brief get the a reference to the configuration of this instrument
  /// \return reference to the configuration
  Config &getConfiguration() { return DreamConfiguration; }

  /// \brief Get const reference to the active geometry
  const Geometry &getGeometry() const;

  DataParser DreamParser{Counters};
};

template <int Type_t>
DreamInstrument<Type_t>::DreamInstrument(Statistics &Stats, struct Counters &counters,
                                 BaseSettings &settings,
                                 EV44Serializer &serializer,
                                 ESSReadout::Parser &essHeaderParser)
    : Counters(counters), Settings(settings),
      Serializer(serializer), ESSHeaderParser(essHeaderParser) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  DreamConfiguration = Config(Settings.ConfigFile);

  DreamConfiguration.loadAndApply();

  ESSHeaderParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeDiffNS);

  if (DreamConfiguration.Instance == Config::DREAM) {
    if (Type_t != DetectorType::DREAM) {
     throw std::runtime_error("Configuration is incompatible with application. This EFU support DREAM");
    }
    Geom = std::make_unique<DreamGeometry>(Stats, DreamConfiguration);
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    if (Type_t != DetectorType::MAGIC) {
     throw std::runtime_error("Configuration is incompatible with application. This EFU support MAGIC");
    }
    Geom = std::make_unique<MagicGeometry>(Stats, DreamConfiguration);
  } else if (DreamConfiguration.Instance == Config::HEIMDAL) {
    if (Type_t != DetectorType::HEIMDAL) {
     throw std::runtime_error("Configuration is incompatible with application. This EFU support HEIMDAL");
    }
    Geom = std::make_unique<HeimdalGeometry>(Stats, DreamConfiguration);
  } else {
    throw std::runtime_error(
        "Unsupported instrument instance (not DREAM/MAGIC/HEIMDAL)");
  }
}

template <int Type_t>
void DreamInstrument<Type_t>::processReadouts() {

  Serializer.checkAndSetReferenceTime(
      ESSHeaderParser.Packet.Time.getRefTimeUInt64());

  /// Traverse readouts, calculate pixels
  for (auto &Data : DreamParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Data.FiberId / 2, Data.FENId);

    // Validate Ring, FEN and configuration through geometry class
    if (!Geom->validateReadoutData(Data)) {
      continue;
    }

    auto TimeOfFlight = ESSHeaderParser.Packet.Time.getTOF(
        ESSTime(Data.TimeHigh, Data.TimeLow));

    if (!TimeOfFlight.has_value()) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      continue;
    }  

    // Calculate pixelid and apply calibration using polymorphism
    // The geometry extracts ModuleParms from the RMConfig array internally
    uint32_t PixelId = Geom->calcPixel(Data);
    XTRACE(DATA, DEB, "PixelId: %u", PixelId);

    if (PixelId != 0) {
      Serializer.addEvent(TimeOfFlight.value(), PixelId);
      Counters.Events++;
    }
  }
}

template <int Type_t>
const Geometry &DreamInstrument<Type_t>::getGeometry() const {
  if (!Geom) {
    throw std::runtime_error(
        "Geometry not initialized for this instrument type");
  }
  return *Geom.get();
}

} // namespace Dream
