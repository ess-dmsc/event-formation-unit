// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Bifrost processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <bifrost/BifrostInstrument.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Bifrost {

BifrostInstrument::BifrostInstrument(struct Counters &counters,
                                     BifrostSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  // XTRACE(INIT, ALW, "Loading configuration file %s",
  //        ModuleSettings.ConfigFile.c_str());
  // BifrostConfiguration = Config(ModuleSettings.ConfigFile);

  if (not ModuleSettings.FilePrefix.empty()) {
    DumpFile = ReadoutFile::create(ModuleSettings.FilePrefix + "bifrost_" +
                                   timeString());
  }

  // ESSReadoutParser.setMaxPulseTimeDiff(BifrostConfiguration.MaxPulseTimeNS);
  // ESSReadoutParser.Packet.Time.setMaxTOF(BifrostConfiguration.MaxTOFNS);
  ESSReadoutParser.Packet.Time.setMaxTOF(0xFFFFFFFFFFFFFFFFULL);
}

BifrostInstrument::~BifrostInstrument() {}

uint32_t BifrostInstrument::calcPixel(int Ring, int Tube, int AmpA, int AmpB) {
  int xoff = geom.xOffset(Ring, Tube);
  int yoff = geom.yOffset(Tube);
  int xlocal = geom.xCoord(AmpA, AmpB);
  int ylocal = geom.yCoord(AmpA, AmpB);
  uint32_t pixel = lgeom.pixel2D(xoff + xlocal, yoff + ylocal);

  XTRACE(DATA, DEB, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
         xoff, xlocal, yoff, ylocal, pixel);

  return pixel;
}

void BifrostInstrument::dumpReadoutToFile(DataParser::BifrostReadout &Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
  CurrentReadout.AmpA = Data.AmpA;
  CurrentReadout.AmpB = Data.AmpB;
  CurrentReadout.RingId = Data.RingId;
  CurrentReadout.FENId = Data.FENId;
  CurrentReadout.TubeId = Data.TubeId;
  DumpFile->push(CurrentReadout);
}

void BifrostInstrument::processReadouts() {
  Serializer->checkAndSetPulseTime(
      ESSReadoutParser.Packet.Time
          .TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  /// Traverse readouts, calculate pixels
  for (auto &Section : BifrostParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Section.RingId, Section.FENId,
           Section.TubeId);

    if (Section.RingId > BifrostConfiguration.MaxValidRing) {
      XTRACE(DATA, WAR, "RING %d is incompatible with config", Section.RingId);
      counters.RingErrors++;
      continue;
    }

    auto &Data = Section;

    if (DumpFile) {
      dumpReadoutToFile(Data);
    }

    // Calculate TOF in ns
    auto TimeOfFlight =
        ESSReadoutParser.Packet.Time.getTOF(Data.TimeHigh, Data.TimeLow, 0);

    XTRACE(DATA, DEB, "PulseTime     %" PRIu64 ", TimeStamp %" PRIu64 " ",
           ESSReadoutParser.Packet.Time.TimeInNS,
           ESSReadoutParser.Packet.Time.toNS(Data.TimeHigh, Data.TimeLow));
    XTRACE(DATA, DEB, "PrevPulseTime %" PRIu64 ", TimeStamp %" PRIu64 " ",
           ESSReadoutParser.Packet.Time.PrevTimeInNS,
           ESSReadoutParser.Packet.Time.toNS(Data.TimeHigh, Data.TimeLow));

    if (TimeOfFlight == ESSReadoutParser.Packet.Time.InvalidTOF) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      continue;
    }

    XTRACE(DATA, DEB, "  Data: time (%10u, %10u) tof %llu, Tube %u, A %d, B %d",
           Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.TubeId, Data.AmpA,
           Data.AmpB);

    // Calculate pixelid and apply calibration
    uint32_t PixelId =
        calcPixel(Data.RingId, Data.TubeId, Data.AmpA, Data.AmpB);

    if (PixelId == 0) {
      counters.PixelErrors++;
    } else {
      counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
    }
  } // for()
}
} // namespace Bifrost
