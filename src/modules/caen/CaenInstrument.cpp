// Copyright (C) 2020 - 2024 European Spallation Source, ERIC. See LICENSE file
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

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

using namespace ESSReadout;

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
CaenInstrument::CaenInstrument(struct CaenCounters &counters,
                               BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  CaenConfiguration = Config(Settings.ConfigFile);
  CaenConfiguration.parseConfig();

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
    throw std::runtime_error("Calibration file is required, none supplied");
  }

  XTRACE(INIT, ALW, "Loading calibration file %s", Settings.CalibFile.c_str());
  Geom->CaenCDCalibration =
      CDCalibration(settings.DetectorName, Settings.CalibFile);
  Geom->CaenCDCalibration.parseCalibration();

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
}

CaenInstrument::~CaenInstrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// caen panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t CaenInstrument::calcPixel(DataParser::CaenReadout &Data) {
  XTRACE(DATA, DEB, "Calculating pixel");

  uint32_t pixel = Geom->calcPixel(Data);
  // seems to be wrong
  // counters.ReadoutsBadAmpl = *Geom->Stats.AmplitudeZero;
  XTRACE(DATA, DEB, "Calculated pixel to be %u", pixel);
  return pixel;
}

void CaenInstrument::dumpReadoutToFile(DataParser::CaenReadout &Data) {
  Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr.getPulseHigh();
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr.getPulseLow();
  CurrentReadout.PrevPulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr.getPrevPulseHigh();
  CurrentReadout.PrevPulseTimeLow =
      ESSReadoutParser.Packet.HeaderPtr.getPrevPulseLow();
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.DataSeqNum = Data.DataSeqNum;
  CurrentReadout.OutputQueue =
      ESSReadoutParser.Packet.HeaderPtr.getOutputQueue();
  CurrentReadout.AmpA = Data.AmpA;
  CurrentReadout.AmpB = Data.AmpB;
  CurrentReadout.AmpC = Data.AmpC;
  CurrentReadout.AmpD = Data.AmpD;
  CurrentReadout.FiberId = Data.FiberId;
  CurrentReadout.FENId = Data.FENId;
  CurrentReadout.Group = Data.Group;
  DumpFile->push(CurrentReadout);
}

void CaenInstrument::processReadouts() {
  XTRACE(DATA, DEB, "Reference time is %" PRIi64,
         ESSReadoutParser.Packet.Time.getRefTimeUInt64());
  /// \todo sometimes PrevPulseTime maybe?
  auto packet_ref_time = static_cast<int64_t>(ESSReadoutParser.Packet.Time.getRefTimeUInt64());
  for (auto & Serializer: Serializers) Serializer->checkAndSetReferenceTime(packet_ref_time);

  /// Traverse readouts, calculate pixels
  for (auto &Data : CaenParser.Result) {
    XTRACE(DATA, DEB, "Fiber %u, FEN %u", Data.FiberId, Data.FENId);
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
        ESSTime(Data.TimeHigh, Data.TimeLow));

    XTRACE(DATA, DEB,
           "PulseTime     %" PRIu64 ", Previous PulseTime: %" PRIu64
           ", Calculated ToF %" PRIu64 " ",
           ESSReadoutParser.Packet.Time.getRefTimeUInt64(),
           ESSReadoutParser.Packet.Time.getPrevRefTimeUInt64(), TimeOfFlight);

    if (TimeOfFlight == ESSReadoutParser.Packet.Time.InvalidTOF) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      continue;
    }

    XTRACE(DATA, DEB,
           "  Data: time (%10u, %10u) tof %llu, SeqNo %u, Group %u, A %d, B "
           "%d, C %d, D %d",
           Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.DataSeqNum,
           Data.Group, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

    // Calculate pixel and apply calibration
    uint32_t PixelId = calcPixel(Data);

    // Determine the correct serializer for this pixel
    auto SerializerId = Geom->calcSerializer(Data);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Pixel error");
      counters.PixelErrors++;
    } else if (SerializerId >= Serializers.size()) {
      XTRACE(EVENT, WAR, "Serializer identification error");
      counters.SerializerErrors++;
    } else {
      XTRACE(EVENT, DEB, "Pixel %u, TOF %u", PixelId, TimeOfFlight);
      Serializers[SerializerId]->addEvent(static_cast<int32_t>(TimeOfFlight), static_cast<int32_t>(PixelId));
      counters.Events++;
    }

  } // for()
}

} // namespace Caen
