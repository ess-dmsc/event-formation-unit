// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Caen processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <caen/CaenInstrument.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <fmt/format.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
CaenInstrument::CaenInstrument(struct Counters &counters,
                               BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  CaenConfiguration = Config(Settings.ConfigFile);

  if (settings.DetectorName == "loki") {
    Geom = new LokiGeometry(CaenConfiguration);
  } else if (settings.DetectorName == "bifrost") {
    Geom = new BifrostGeometry(CaenConfiguration);
  } else if (settings.DetectorName == "miracles") {
      Geom = new MiraclesGeometry(CaenConfiguration);
  } else if (settings.DetectorName == "cspec") {
      Geom = new CspecGeometry(CaenConfiguration);
  } else {
    XTRACE(INIT, ERR, "Invalid Detector Name %s",
           settings.DetectorName.c_str());
    throw std::runtime_error(
        fmt::format("Invalid Detector Name {}", settings.DetectorName));
  }

  if (Settings.CalibFile.empty()) {
    XTRACE(INIT, ALW, "Using the identity 'calibration'");
    uint32_t MaxPixels = Geom->ESSGeom->max_pixel();
    uint32_t Straws = MaxPixels / CaenConfiguration.Resolution;
    XTRACE(INIT, DEB,
           "Calculating Straws, MaxPixels: %u, Resolution: %u, Straws: %u",
           MaxPixels, CaenConfiguration.Resolution, Straws);

    XTRACE(INIT, ALW, "Inst: Straws: %u, Resolution: %u", Straws,
           CaenConfiguration.Resolution);
    Geom->CaenCalibration.nullCalibration(Straws, CaenConfiguration.Resolution);
  } else {
    XTRACE(INIT, ALW, "Loading calibration file %s",
           Settings.CalibFile.c_str());
    Geom->CaenCalibration = Calibration(Settings.CalibFile);
  }

  if (Geom->CaenCalibration.getMaxPixel() != Geom->ESSGeom->max_pixel()) {
    XTRACE(INIT, ALW, "Config pixels: %u, calib pixels: %u",
           Geom->ESSGeom->max_pixel(), Geom->CaenCalibration.getMaxPixel());
    LOG(PROCESS, Sev::Error, "Error: pixel mismatch Config ({}) and Calib ({})",
        Geom->ESSGeom->max_pixel(), Geom->CaenCalibration.getMaxPixel());
    throw std::runtime_error("Pixel mismatch");
  }

  if (not Settings.DumpFilePrefix.empty()) {
    if (boost::filesystem::path(Settings.DumpFilePrefix).has_extension()) {

      DumpFile =
          ReadoutFile::create(boost::filesystem::path(Settings.DumpFilePrefix)
                                  .replace_extension(""));
    } else {
      DumpFile =
          ReadoutFile::create(Settings.DumpFilePrefix + "_" + timeString());
    }
  }

  ESSReadoutParser.setMaxPulseTimeDiff(CaenConfiguration.MaxPulseTimeNS);
  ESSReadoutParser.Packet.Time.setMaxTOF(CaenConfiguration.MaxTOFNS);

  Geom->CaenCalibration.Stats.ClampLow = &counters.ReadoutsClampLow;
  Geom->CaenCalibration.Stats.ClampHigh = &counters.ReadoutsClampHigh;
  Geom->Stats.FENErrors = &counters.FENErrors;
  Geom->Stats.RingErrors = &counters.RingErrors;
  Geom->Stats.TubeErrors = &counters.TubeErrors;
}

CaenInstrument::~CaenInstrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// caen panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t CaenInstrument::calcPixel(DataParser::CaenReadout &Data) {
  XTRACE(DATA, DEB, "Calculating pixel");

  uint32_t pixel = Geom->calcPixel(Data);
  counters.ReadoutsBadAmpl = Geom->Stats.AmplitudeZero;
  XTRACE(DATA, DEB, "Calculated pixel to be %u", pixel);
  return pixel;
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
  XTRACE(DATA, DEB, "Reference time is %" PRIi64,
         ESSReadoutParser.Packet.Time.TimeInNS);
  /// \todo sometimes PrevPulseTime maybe?
  Serializer->checkAndSetReferenceTime(ESSReadoutParser.Packet.Time.TimeInNS);
  SerializerII->checkAndSetReferenceTime(ESSReadoutParser.Packet.Time.TimeInNS);

  /// Traverse readouts, calculate pixels
  for (auto &Data : CaenParser.Result) {
    XTRACE(DATA, DEB, "Ring %u, FEN %u", Data.RingId, Data.FENId);
    bool validData = Geom->validateData(Data);
    if (not validData) {
      XTRACE(DATA, WAR, "Invalid Data, skipping readout");
      continue;
    }

    if (DumpFile) {
      dumpReadoutToFile(Data);
    }

    // Calculate TOF in ns
    uint64_t TimeOfFlight = ESSReadoutParser.Packet.Time.getTOF(
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
    uint32_t PixelId = calcPixel(Data);

    if (PixelId == 0) {
      XTRACE(DATA, ERR, "Pixel error");
      counters.PixelErrors++;
    } else {
      XTRACE(DATA, DEB, "Valid data, adding to serializer");
      Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
      SerializerII->addEvent(Data.AmpA + Data.AmpB + Data.AmpC + Data.AmpD, 0);
    }

  } // for()
}

} // namespace Caen
