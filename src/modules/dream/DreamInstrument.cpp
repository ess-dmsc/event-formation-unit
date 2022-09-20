// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/time/TimeString.h>
#include <common/debug/Trace.h>
#include <dream/DreamInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

DreamInstrument::DreamInstrument(struct Counters &counters,
                                 DreamSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  DreamConfiguration = Config(ModuleSettings.ConfigFile);

  DreamConfiguration.loadAndApply();


  ESSReadoutParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeNS);
}

uint32_t DreamInstrument::calcPixel(uint8_t Sector, uint8_t Sumo, uint8_t Cassette,
                                    uint8_t Counter, uint8_t Wire, uint8_t Strip) {
return EcGeom.getPixel(Sector, Sumo, Cassette, Counter, Wire, Strip);
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
  for (auto &Section : DreamParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

    for (auto &Data : Section.Data) {
      auto TimeOfFlight = Time.getTOF(0, Data.Tof); // TOF in ns

      XTRACE(DATA, DEB,
             "  Data: time (0, %u), sector %u, sumo %u, cassette %u, counter %u, wire "
             "%u, strip %u",
             Data.Tof, Data.Sector, Data.Sumo, Data.Cassette,
             Data.Counter, Data.Wire, Data.Strip);

      // Calculate pixelid and apply calibration
      uint32_t PixelId = calcPixel(Data.Sector, Data.Sumo, Data.Cassette,
                                   Data.Counter, Data.Wire, Data.Strip);

      if (PixelId == 0) {
        counters.GeometryErrors++;
      } else {
        counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
        counters.Events++;
      }
    }
  }
}

//
DreamInstrument::~DreamInstrument() {}

} // namespace Dream
