// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <dream/DreamInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

DreamInstrument::DreamInstrument(struct Counters &counters,
                                 BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  DreamConfiguration = Config(Settings.ConfigFile);

  DreamConfiguration.loadAndApply();

  ESSReadoutParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeDiffNS);

  if (DreamConfiguration.Instance == Config::DREAM) {
    Type = ESSReadout::Parser::DREAM;
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    Type = ESSReadout::Parser::MAGIC;
  } else {
    throw std::runtime_error(
        "Unsupported instrument instance (not DREAM/MAGIC)");
  }
}

uint32_t DreamInstrument::calcPixel(Config::ModuleParms &Parms,
                                    DataParser::DreamReadout &Data) {
  if (DreamConfiguration.Instance == Config::DREAM) {
    return DreamGeom.getPixel(Parms, Data);
  } else if (DreamConfiguration.Instance == Config::MAGIC) {
    return MagicGeom.getPixel(Parms, Data);
  } else {
    return 0;
  }
}

void DreamInstrument::processReadouts() {
  /// \todo We have a design issue here. Check is it a good approach to share
  /// the ownership of the buffer between the parser and the instrument.
  auto PacketHeader = ESSReadoutParser.Packet.HeaderPtr;
  uint64_t PulseTime = Time.setReference(PacketHeader->getPulseHigh(),
                                         PacketHeader->getPulseLow());
  uint64_t PrevPulseTime = Time.setPrevReference(
      PacketHeader->getPrevPulseHigh(), PacketHeader->getPrevPulseLow());

  if (PulseTime - PrevPulseTime > DreamConfiguration.MaxPulseTimeDiffNS) {
    XTRACE(DATA, WAR, "PulseTime and PrevPulseTime too far apart: %" PRIu64 "",
           (PulseTime - PrevPulseTime));
    counters.ReadoutStats.ErrorTimeHigh++;
    counters.ErrorESSHeaders++;
    return;
  }

  Serializer->checkAndSetReferenceTime(
      PulseTime); /// \todo sometimes PrevPulseTime maybe?
  XTRACE(DATA, DEB, "PulseTime     (%u,%u)", PacketHeader->getPulseHigh(),
         PacketHeader->getPulseLow());
  XTRACE(DATA, DEB, "PrevPulseTime (%u,%u)", PacketHeader->getPrevPulseHigh(),
         PacketHeader->getPrevPulseLow());
  //

  /// Traverse readouts, calculate pixels
  for (auto &Data : DreamParser.Result) {
    int Ring = Data.FiberId / 2;
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Ring, Data.FENId);

    if (Ring > DreamConfiguration.MaxRing) {
      XTRACE(DATA, WAR, "Invalid RING: %u", Ring);
      counters.RingMappingErrors++;
      continue;
    }

    if (Data.FENId > DreamConfiguration.MaxFEN) {
      XTRACE(DATA, WAR, "Invalid FEN: %u", Data.FENId);
      counters.FENMappingErrors++;
      continue;
    }

    Config::ModuleParms &Parms = DreamConfiguration.RMConfig[Ring][Data.FENId];

    if (not Parms.Initialised) {
      XTRACE(DATA, WAR, "Config mismatch: RING %u, FEN %u is unconfigured",
             Ring, Data.FENId);
      counters.ConfigErrors++;
      continue;
    }

    auto TimeOfFlight =
        ESSReadoutParser.Packet.Time.getTOF(Data.TimeHigh, Data.TimeLow);

    // Calculate pixelid and apply calibration
    uint32_t PixelId = calcPixel(Parms, Data);
    XTRACE(DATA, DEB, "PixelId: %u", PixelId);

    if (PixelId == 0) {
      counters.GeometryErrors++;
    } else {
      counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
    }
  }
}

//
DreamInstrument::~DreamInstrument() {}

} // namespace Dream
