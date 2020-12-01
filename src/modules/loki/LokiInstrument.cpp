// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Loki processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <loki/LokiInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
LokiInstrument::LokiInstrument(struct Counters & counters,
    LokiSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {

    XTRACE(INIT, ALW, "Loading configuration file %s",
      ModuleSettings.ConfigFile.c_str());
    LokiConfiguration = Config(ModuleSettings.ConfigFile);

    Amp2Pos.setResolution(LokiConfiguration.Resolution);

    if (ModuleSettings.CalibFile.empty()) {
      XTRACE(INIT, ALW, "Using the identity 'calibration'");
      uint32_t MaxPixels = LokiConfiguration.getMaxPixel();
      uint32_t Straws = MaxPixels/LokiConfiguration.Resolution;
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
}


/// \brief helper function to calculate pixels from knowledge about
/// loki panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t LokiInstrument::calcPixel(PanelGeometry & Panel, uint8_t FEN,
    DataParser::LokiReadout & Data) {

  uint8_t TubeGroup = FEN - 1;
  uint8_t LocalTube = Data.TubeId;

  bool valid = Amp2Pos.calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

  if (not valid) {
    return 0;
  }

  auto Straw = Amp2Pos.StrawId;
  /// Position (and CalibratedPos) are per definition == X
  uint16_t Position = Amp2Pos.PosId; // position along the straw

  /// Globalstraw is per its definition == Y
  uint32_t GlobalStraw = Panel.getGlobalStrawId(TubeGroup, LocalTube, Straw);

  uint16_t CalibratedPos = LokiCalibration.strawCorrection(GlobalStraw, Position);

  uint32_t PixelId = LokiConfiguration.Geometry->pixel2D(CalibratedPos,
    GlobalStraw);

  XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u",
         Position, CalibratedPos, GlobalStraw, PixelId);

  return PixelId;
}


void LokiInstrument::dumpReadoutToFile(DataParser::ParsedData & Section,
    DataParser::LokiReadout & Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.DataSeqNum = Data.DataSeqNum;
  CurrentReadout.AmpA = Data.AmpA;
  CurrentReadout.AmpB = Data.AmpB;
  CurrentReadout.AmpC = Data.AmpC;
  CurrentReadout.AmpD = Data.AmpD;
  CurrentReadout.RingId = Section.RingId;
  CurrentReadout.FENId = Section.FENId;
  CurrentReadout.TubeId = Data.TubeId;
  DumpFile->push(CurrentReadout);
}

} // namespace
