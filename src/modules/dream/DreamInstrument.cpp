// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include "common/Statistics.h"
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/kafka/EV44Serializer.h>
#include <common/time/TimeString.h>
#include <dream/DreamInstrument.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/geometry/Geometry.h>
#include <dream/geometry/HeimdalGeometry.h>
#include <dream/geometry/MagicGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

using namespace esstime;
using namespace ESSReadout;

DreamInstrument::DreamInstrument(Statistics &Stats, struct Counters &counters,
                                 BaseSettings &settings,
                                 EV44Serializer &serializer,
                                 ESSReadout::Parser &essHeaderParser)
    : Stats(Stats), Counters(counters), Settings(settings),
      Serializer(serializer), ESSHeaderParser(essHeaderParser) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  DreamConfiguration = Config(Settings.ConfigFile);

  DreamConfiguration.loadAndApply();

  ESSHeaderParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeDiffNS);

  if (DreamConfiguration.Instance == Config::DREAM) {
    Geom = std::make_unique<DreamGeometry>(Stats, DreamConfiguration);
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    Geom = std::make_unique<MagicGeometry>(Stats, DreamConfiguration);
  } else if (DreamConfiguration.Instance == Config::HEIMDAL) {
    Geom = std::make_unique<HeimdalGeometry>(Stats, DreamConfiguration);
  } else {
    throw std::runtime_error(
        "Unsupported instrument instance (not DREAM/MAGIC/HEIMDAL)");
  }
}

void DreamInstrument::processReadouts() {

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

    // Calculate pixelid and apply calibration using polymorphism
    // The geometry extracts ModuleParms from the RMConfig array internally
    uint32_t PixelId = Geom->calcPixel<DataParser::CDTReadout>(Data);
    XTRACE(DATA, DEB, "PixelId: %u", PixelId);

    if (PixelId != 0) {
      Serializer.addEvent(TimeOfFlight, PixelId);
      Counters.Events++;
    }
  }
}

//
DreamInstrument::~DreamInstrument() {}

const Geometry &DreamInstrument::getGeometry() const {
  if (!Geom) {
    throw std::runtime_error(
        "Geometry not initialized for this instrument type");
  }
  return *Geom.get();
}

} // namespace Dream
