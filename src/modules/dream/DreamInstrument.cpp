// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
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
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

DreamInstrument::DreamInstrument(struct Counters &counters,
                                 BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  DreamConfiguration = Config(Settings.ConfigFile);

  DreamConfiguration.loadAndApply();

  ESSReadoutParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeNS);
}

uint32_t DreamInstrument::calcPixel(Config::ModuleParms &Parms,
                                    DataParser::DreamReadout &Data) {
  return Geometry.getPixel(Parms, Data);
}

void DreamInstrument::processReadouts() {
  auto PacketHeader = ESSReadoutParser.Packet.HeaderPtr;
  uint64_t PulseTime =
      Time.setReference(PacketHeader->PulseHigh, PacketHeader->PulseLow);
  uint64_t __attribute__((unused)) PrevPulseTime = Time.setPrevReference(
      PacketHeader->PrevPulseHigh, PacketHeader->PrevPulseLow);

  /// \todo Add Dream config to handle max time between pulses
  /// for now arbitrarily use 5 seconds
  if (PulseTime - PrevPulseTime > 5 * 1'000'000'000ULL) {
    XTRACE(DATA, WAR, "PulseTime and PrevPulseTime too far apart: %" PRIu64 "",
           (PulseTime - PrevPulseTime));
    counters.ReadoutStats.ErrorTimeHigh++;
    counters.ErrorESSHeaders++;
    return;
  }

  Serializer->pulseTime(PulseTime); /// \todo sometimes PrevPulseTime maybe?
  XTRACE(DATA, DEB, "PulseTime     (%u,%u)", PacketHeader->PulseHigh,
         PacketHeader->PulseLow);
  XTRACE(DATA, DEB, "PrevPulseTime (%u,%u)", PacketHeader->PrevPulseHigh,
         PacketHeader->PrevPulseLow);
  //

  /// Traverse readouts, calculate pixels
  for (auto &Data : DreamParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Data.RingId, Data.FENId);

    if (Data.RingId > DreamConfiguration.MaxRing) {
      XTRACE(DATA, WAR, "Invalid RING: %u", Data.RingId);
      counters.RingErrors++;
      continue;
    }

    if (Data.FENId > DreamConfiguration.MaxFEN) {
      XTRACE(DATA, WAR, "Invalid FEN: %u", Data.FENId);
      counters.FENErrors++;
      continue;
    }

    Config::ModuleParms &Parms =
        DreamConfiguration.RMConfig[Data.RingId][Data.FENId];

    if (not Parms.Initialised) {
      XTRACE(DATA, WAR, "Config mismatch: RING %u, FEN %u is unconfigured",
             Data.RingId, Data.FENId);
      counters.ConfigErrors++;
      continue;
    }

    auto TimeOfFlight =
        ESSReadoutParser.Packet.Time.getTOF(Data.TimeHigh, Data.TimeLow);

    // Calculate pixelid and apply calibration
    uint32_t PixelId = calcPixel(Parms, Data);

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
