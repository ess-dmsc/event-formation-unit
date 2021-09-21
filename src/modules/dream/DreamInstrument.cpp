// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/TimeString.h>
#include <common/Trace.h>
#include <dream/DreamInstrument.h>
#include <dream/geometry/DreamGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Dream {

DreamInstrument::DreamInstrument(struct Counters &counters,
                                 DreamSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  DreamConfiguration = Config(ModuleSettings.ConfigFile);


  ESSReadoutParser.setMaxPulseTimeDiff(DreamConfiguration.MaxPulseTimeNS);
}

uint32_t DreamInstrument::calcPixel(uint8_t Sector, uint8_t Sumo, uint8_t Strip,
                                    uint8_t Wire, uint8_t Cassette,
                                    uint8_t Counter) {
  DreamGeometry::EndCapParams endcap = {Sector, Sumo, Strip, Wire, Cassette, Counter};

  uint32_t Pixel{0};
  DreamGeometry::PixelIdFromEndCapParams(endcap, Pixel);
  return Pixel;
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

    if (Section.FENId == 0) {
      XTRACE(DATA, WAR, "FENId == 0");
      counters.MappingErrors++;
      continue;
    }

    for (auto &Data : Section.Data) {
      auto TimeOfFlight = Time.getTOF(0, Data.Tof); // TOF in ns

      XTRACE(DATA, DEB,
             "  Data: time (0, %u), mod %u, sumo %u, strip %u, wire %u, seg "
             "%u, ctr %u",
             Data.Tof, Data.Module, Data.Sumo, Data.Strip, Data.Wire,
             Data.Segment, Data.Counter);

      // Calculate pixelid and apply calibration
      uint32_t PixelId = calcPixel(Data.Module, Data.Sumo, Data.Strip,
                                   Data.Wire, Data.Segment, Data.Counter);

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
