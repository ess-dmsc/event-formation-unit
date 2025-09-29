// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/kafka/EV44Serializer.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <dream/DreamInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

using namespace esstime;
using namespace ESSReadout;

DreamInstrument::DreamInstrument(struct Counters &counters, BaseSettings &settings,
                                 EV44Serializer &serializer,
                                 ESSReadout::Parser &essHeaderParser)
    : Counters(counters), Settings(settings), Serializer(serializer),
      ESSHeaderParser(essHeaderParser) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  DreamConfiguration = Config(Settings.ConfigFile);

  DreamConfiguration.loadAndApply();

  ESSHeaderParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeDiffNS);

  if (DreamConfiguration.Instance == Config::DREAM) {
    Type = DetectorType::DREAM;
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    Type = DetectorType::MAGIC;
  } else if (DreamConfiguration.Instance == Config::HEIMDAL) {
    Type = DetectorType::HEIMDAL;
  } else {
    throw std::runtime_error(
        "Unsupported instrument instance (not DREAM/MAGIC)");
  }
}

uint32_t DreamInstrument::calcPixel(Config::ModuleParms &Parms,
                                    DataParser::CDTReadout &Data) {
  if (DreamConfiguration.Instance == Config::DREAM) {
    return DreamGeom.getPixel(Parms, Data);
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    return MagicGeom.getPixel(Parms, Data);
  } else if (DreamConfiguration.Instance == Config::HEIMDAL) {
    return HeimdalGeom.getPixel(Parms, Data);
  } else {
    return 0;
  }
}

void DreamInstrument::processReadouts() {

  Serializer.checkAndSetReferenceTime(
      ESSHeaderParser.Packet.Time.getRefTimeUInt64());

  /// Traverse readouts, calculate pixels
  for (auto &Data : DreamParser.Result) {
    int Ring = Data.FiberId / 2;
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Ring, Data.FENId);

    if (Ring > DreamConfiguration.MaxRing) {
      XTRACE(DATA, WAR, "Invalid RING: %u", Ring);
      Counters.RingMappingErrors++;
      continue;
    }

    if (Data.FENId > DreamConfiguration.MaxFEN) {
      XTRACE(DATA, WAR, "Invalid FEN: %u", Data.FENId);
      Counters.FENMappingErrors++;
      continue;
    }

    Config::ModuleParms &Parms = DreamConfiguration.RMConfig[Ring][Data.FENId];

    if (not Parms.Initialised) {
      XTRACE(DATA, WAR, "Config mismatch: RING %u, FEN %u is unconfigured",
             Ring, Data.FENId);
      Counters.ConfigErrors++;
      continue;
    }

    auto TimeOfFlight = ESSHeaderParser.Packet.Time.getTOF(
        ESSTime(Data.TimeHigh, Data.TimeLow));

    // Calculate pixelid and apply calibration
    uint32_t PixelId = calcPixel(Parms, Data);
    XTRACE(DATA, DEB, "PixelId: %u", PixelId);

    if (PixelId == 0) {
      Counters.GeometryErrors++;
    } else {
      Serializer.addEvent(TimeOfFlight, PixelId);
      Counters.Events++;
    }
  }
}

//
DreamInstrument::~DreamInstrument() {}

} // namespace Dream
