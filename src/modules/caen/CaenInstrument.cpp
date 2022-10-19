// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Caen processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <caen/CaenInstrument.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
CaenInstrument::CaenInstrument(struct Counters &counters,
                               CaenSettings &moduleSettings)
    : counters(counters), ModuleSettings(moduleSettings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  CaenConfiguration = Config(ModuleSettings.ConfigFile);

  LokiGeom.setResolution(CaenConfiguration.Resolution);

  if (ModuleSettings.CalibFile.empty()) {
    XTRACE(INIT, ALW, "Using the identity 'calibration'");
    uint32_t MaxPixels = CaenConfiguration.getMaxPixel();
    uint32_t Straws = MaxPixels / CaenConfiguration.Resolution;

    XTRACE(INIT, ALW, "Inst: Straws: %u, Resolution: %u", Straws,
           CaenConfiguration.Resolution);
    CaenCalibration.nullCalibration(Straws, CaenConfiguration.Resolution);
  } else {
    XTRACE(INIT, ALW, "Loading calibration file %s",
           ModuleSettings.CalibFile.c_str());
    CaenCalibration = Calibration(ModuleSettings.CalibFile);
  }
  LokiGeom.setCalibration(CaenCalibration);

  if (CaenCalibration.getMaxPixel() != CaenConfiguration.getMaxPixel()) {
    XTRACE(INIT, ALW, "Config pixels: %u, calib pixels: %u",
           CaenConfiguration.getMaxPixel(), CaenCalibration.getMaxPixel());
    LOG(PROCESS, Sev::Error, "Error: pixel mismatch Config ({}) and Calib ({})",
        CaenConfiguration.getMaxPixel(), CaenCalibration.getMaxPixel());
    throw std::runtime_error("Pixel mismatch");
  }

  if (!ModuleSettings.FilePrefix.empty()) {
    DumpFile =
        ReadoutFile::create(ModuleSettings.FilePrefix + "caen_" + timeString());
  }

  ESSReadoutParser.setMaxPulseTimeDiff(CaenConfiguration.MaxPulseTimeNS);
  ESSReadoutParser.Packet.Time.setMaxTOF(CaenConfiguration.MaxTOFNS);
}

CaenInstrument::~CaenInstrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// caen panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t CaenInstrument::calcPixel(PanelGeometry &Panel, uint8_t FEN,
                                   DataParser::CaenReadout &Data) {
  if (CaenConfiguration.InstrumentName == "LoKI"){
    auto pixel = LokiGeom.calcPixel(Panel, FEN, Data);
    counters.ReadoutsBadAmpl = LokiGeom.Stats.AmplitudeZero;
    counters.OutsideRegion = LokiGeom.Stats.OutsideRegion;
    return pixel;
  }
  return 0;
  }


void CaenInstrument::dumpReadoutToFile(DataParser::CaenReadout &Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
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

void CaenInstrument::processReadouts() {
  Serializer->checkAndSetReferenceTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?
  SerializerII->checkAndSetReferenceTime(ESSReadoutParser.Packet.Time.TimeInNS);

  /// Traverse readouts, calculate pixels
  for (auto &Section : CaenParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

    if (Section.RingId >= CaenConfiguration.Panels.size()) {
      XTRACE(DATA, WAR, "RINGId %d is incompatible with #panels: %d",
             Section.RingId, CaenConfiguration.Panels.size());
      counters.RingErrors++;
      continue;
    }

    PanelGeometry &Panel = CaenConfiguration.Panels[Section.RingId];

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
    auto TimeOfFlight = ESSReadoutParser.Packet.Time.getTOF(
        Data.TimeHigh, Data.TimeLow, CaenConfiguration.ReadoutConstDelayNS);

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
      Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
      SerializerII->addEvent(Data.AmpA + Data.AmpB + Data.AmpC + Data.AmpD, 0);
    }

  } // for()
  counters.ReadoutsClampLow = CaenCalibration.Stats.ClampLow;
  counters.ReadoutsClampHigh = CaenCalibration.Stats.ClampHigh;
}

} // namespace Caen
