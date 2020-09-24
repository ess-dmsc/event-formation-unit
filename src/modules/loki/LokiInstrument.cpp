// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Loki processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/TimeString.h>
#include <loki/LokiInstrument.h>

namespace Loki {

LokiInstrument::LokiInstrument(struct Counters & counters,
    LokiSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {

    LokiConfiguration = Config(ModuleSettings.ConfigFile);

    Amp2Pos.setResolution(LokiConfiguration.Resolution);

    if (ModuleSettings.CalibFile.empty()) {
      uint32_t MaxPixels = LokiConfiguration.getMaxPixel();
      LokiCalibration.nullCalibration(MaxPixels);
    } else {
      LokiCalibration = Calibration(ModuleSettings.CalibFile);
    }

    if (LokiCalibration.getMaxPixel() != LokiConfiguration.getMaxPixel()) {
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
  /// \todo validate this assumption from LoKI readout data
  /// FPGAId: 2 bits
  /// TUBE: 1 bit
  /// LocalTube: 0 - 7
  uint8_t LocalTube = ((Data.FPGAId & 0x3) << 1) + (Data.TubeId & 0x1);

  Amp2Pos.calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
  auto Straw = Amp2Pos.StrawId;

  auto XPos = Amp2Pos.PosId;
  auto YPos = Panel.getGlobalStrawId(TubeGroup, LocalTube, Straw);

  uint32_t PixelId = LokiConfiguration.Geometry->pixel2D(XPos,YPos);
  uint32_t MappedPixelId = LokiCalibration.Mapping[PixelId];

  XTRACE(EVENT, DEB, "xpos %u, ypos %u, pixel: %u (calibrated: %u)",
    XPos, YPos, PixelId, MappedPixelId);

  return MappedPixelId;
}


void LokiInstrument::dumpReadoutToFile(DataParser::ParsedData & Section,
    DataParser::LokiReadout & Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.AmpA = Data.AmpA;
  CurrentReadout.AmpB = Data.AmpB;
  CurrentReadout.AmpC = Data.AmpC;
  CurrentReadout.AmpD = Data.AmpD;
  CurrentReadout.RingId = Section.RingId;
  CurrentReadout.FENId = Section.FENId;
  CurrentReadout.FPGAId = Data.FPGAId;
  CurrentReadout.TubeId = Data.TubeId;
  DumpFile->push(CurrentReadout);
}

} // namespace
