// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Miracles processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <miracles/MiraclesInstrument.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Miracles {

MiraclesInstrument::MiraclesInstrument(struct Counters &counters,
                                     MiraclesSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  // XTRACE(INIT, ALW, "Loading configuration file %s",
  //        ModuleSettings.ConfigFile.c_str());
  // MiraclesConfiguration = Config(ModuleSettings.ConfigFile);

  if (not ModuleSettings.FilePrefix.empty()) {
    DumpFile = ReadoutFile::create(ModuleSettings.FilePrefix + "miracles_" +
                                   timeString());
  }

  // ESSReadoutParser.setMaxPulseTimeDiff(MiraclesConfiguration.MaxPulseTimeNS);
  // ESSReadoutParser.Packet.Time.setMaxTOF(MiraclesConfiguration.MaxTOFNS);
  ESSReadoutParser.Packet.Time.setMaxTOF(0xFFFFFFFFFFFFFFFFULL);
}

MiraclesInstrument::~MiraclesInstrument() {}

uint32_t MiraclesInstrument::calcPixel(int Ring, int Tube, int AmpA, int AmpB) {
  int xCoord = geom.xCoord(Ring, Tube, AmpA, AmpB);
  int yCoord = geom.yCoord(Ring, AmpA, AmpB);
  uint32_t pixel = lgeom.pixel2D(xCoord, yCoord);

  XTRACE(DATA, DEB, "xcoord %d, ycoord %d, pixel %hu",
         xCoord, yCoord, pixel);

  return pixel;
}

void MiraclesInstrument::dumpReadoutToFile(DataParser::MiraclesReadout &Data) {
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

void MiraclesInstrument::processReadouts() {
  Serializer->checkAndSetPulseTime(
      ESSReadoutParser.Packet.Time
          .TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  /// Traverse readouts, calculate pixels
  for (auto &Section : MiraclesParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Section.RingId, Section.FENId,
           Section.TubeId);

    if (Section.RingId > MiraclesConfiguration.MaxValidRing) {
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
} // namespace Miracles
