// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Loki processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/time/TimeString.h>
#include <common/debug/Trace.h>
#include <loki/LokiInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_WAR

namespace Loki {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
LokiInstrument::LokiInstrument(struct Counters &counters,
                               LokiSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  LokiConfiguration = Config(ModuleSettings.ConfigFile);

  Amp2Pos.setResolution(LokiConfiguration.Resolution);

  if (ModuleSettings.CalibFile.empty()) {
    XTRACE(INIT, ALW, "Using the identity 'calibration'");
    uint32_t MaxPixels = LokiConfiguration.getMaxPixel();
    uint32_t Straws = MaxPixels / LokiConfiguration.Resolution;

    XTRACE(INIT, ALW, "Inst: Straws: %u, Resolution: %u", Straws,
           LokiConfiguration.Resolution);
    LokiCalibration.nullCalibration(Straws, LokiConfiguration.Resolution);
  } else {
    XTRACE(INIT, ALW, "Loading calibration file %s",
           ModuleSettings.CalibFile.c_str());
    LokiCalibration = Calibration(ModuleSettings.CalibFile);
  }

  if (LokiCalibration.getMaxPixel() != LokiConfiguration.getMaxPixel()) {
    XTRACE(INIT, ALW, "Config pixels: %u, calib pixels: %u",
           LokiConfiguration.getMaxPixel(), LokiCalibration.getMaxPixel());
    LOG(PROCESS, Sev::Error, "Error: pixel mismatch Config ({}) and Calib ({})",
        LokiConfiguration.getMaxPixel(), LokiCalibration.getMaxPixel());
    throw std::runtime_error("Pixel mismatch");
  }

  if (!ModuleSettings.FilePrefix.empty()) {
    DumpFile = ReadoutFile::create(ModuleSettings.FilePrefix + "loki_" + timeString());
  }

  ESSReadoutParser.setMaxPulseTimeDiff(LokiConfiguration.MaxPulseTimeNS);
}

LokiInstrument::~LokiInstrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// loki panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t LokiInstrument::calcPixel(PanelGeometry &Panel, uint8_t FEN,
                                   DataParser::LokiReadout &Data) {

  uint8_t TubeGroup = FEN;
  uint8_t LocalTube = Data.TubeId;

  bool valid = Amp2Pos.calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

  counters.ReadoutsBadAmpl = Amp2Pos.Stats.AmplitudeZero;

  if (not valid) {
    return 0;
  }

  auto Straw = Amp2Pos.StrawId;
  /// Position (and CalibratedPos) are per definition == X
  double Position = Amp2Pos.PosVal; // position along the straw

  /// Globalstraw is per its definition == Y
  uint32_t GlobalStraw = Panel.getGlobalStrawId(TubeGroup, LocalTube, Straw);

  XTRACE(EVENT, DEB, "global straw: %u", GlobalStraw);
  if (GlobalStraw == Panel.StrawError) {
    XTRACE(EVENT, WAR, "Invalid straw id: %d", GlobalStraw);
    return 0;
  }

  if ((GlobalStraw < ModuleSettings.MinStraw) or (GlobalStraw > ModuleSettings.MaxStraw)) {
    counters.OutsideRegion++;
    return 0;
  }

  uint16_t CalibratedPos =
      LokiCalibration.strawCorrection(GlobalStraw, Position);
  XTRACE(EVENT, DEB, "calibrated pos: %u", CalibratedPos);

  uint32_t PixelId =
      LokiConfiguration.Geometry->pixel2D(CalibratedPos, GlobalStraw);

  XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u", Position,
         CalibratedPos, GlobalStraw, PixelId);

  return PixelId;
}

void LokiInstrument::dumpReadoutToFile(DataParser::LokiReadout &Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.DataSeqNum = Data.DataSeqNum;
  CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
  CurrentReadout.AmpA = Data.AmpA;
  CurrentReadout.AmpB = Data.AmpB;
  CurrentReadout.AmpC = Data.AmpC;
  CurrentReadout.AmpD = Data.AmpD;
  CurrentReadout.RingId = Data.RingId;
  CurrentReadout.FENId = Data.FENId;
  CurrentReadout.TubeId = Data.TubeId;
  DumpFile->push(CurrentReadout);
}

void LokiInstrument::processReadouts() {
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?
  SerializerII->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS);

  /// Traverse readouts, calculate pixels
  for (auto &Section : LokiParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

    if (Section.RingId >= LokiConfiguration.Panels.size()) {
      XTRACE(DATA, WAR, "RINGId %d is incompatible with #panels: %d",
             Section.RingId, LokiConfiguration.Panels.size());
      counters.RingErrors++;
      continue;
    }

    PanelGeometry &Panel = LokiConfiguration.Panels[Section.RingId];

    if (Section.FENId >= Panel.getMaxGroup()) {
      XTRACE(DATA, WAR, "FENId %d outside valid range 0 - %d", Section.FENId,
             Panel.getMaxGroup() - 1);
      counters.FENErrors++;
      continue;
    }

    auto &Data = Section;

    if (DumpFile) {
      dumpReadoutToFile(Data);
    }

    // Calculate TOF in ns
    auto TimeOfFlight = ESSReadoutParser.Packet.Time.getTOF(Data.TimeHigh, Data.TimeLow,
                                    LokiConfiguration.ReadoutConstDelayNS);

    if (TimeOfFlight == ESSReadoutParser.Packet.Time.InvalidTOF) {
      TimeOfFlight = ESSReadoutParser.Packet.Time.getPrevTOF(Data.TimeHigh, Data.TimeLow,
                                     LokiConfiguration.ReadoutConstDelayNS);
    }

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

    if (TimeOfFlight >= 800000000) {
      XTRACE(DATA, WAR, "High TOF value. Data: time (%10u, %10u) tof %llu, SeqNo %u, Tube %u, A %d, B "
           "%d, C %d, D %d",
           Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.DataSeqNum,
           Data.TubeId, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
      counters.HighTOF++;
    }

    XTRACE(DATA, DEB,
           "  Data: time (%10u, %10u) tof %llu, SeqNo %u, Tube %u, A %d, B "
           "%d, C %d, D %d",
           Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.DataSeqNum,
           Data.TubeId, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

    // Calculate pixelid and apply calibration
    uint32_t PixelId = calcPixel(Panel, Section.FENId, Data);

    if (PixelId == 0) {
      counters.PixelErrors++;
    } else {
      counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
      SerializerII->addEvent(Data.AmpA + Data.AmpB + Data.AmpC + Data.AmpD, 0);
    }


  } // for()
  counters.ReadoutsClampLow = LokiCalibration.Stats.ClampLow;
  counters.ReadoutsClampHigh = LokiCalibration.Stats.ClampHigh;
}

} // namespace Loki
