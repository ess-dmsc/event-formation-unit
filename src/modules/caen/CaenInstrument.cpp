// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
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
CaenInstrument::CaenInstrument(Statistics &Stats, struct CaenCounters &counters,
                               BaseSettings &settings,
                               ESSReadout::Parser &essHeaderParser)
    : counters(counters), Settings(settings), ESSHeaderParser(essHeaderParser) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  CaenConfiguration = Config(Settings.ConfigFile);
  CaenConfiguration.parseConfig();

  if (settings.DetectorName == "loki") {
    Geom = new LokiGeometry(Stats, CaenConfiguration);
  } else if (settings.DetectorName == "bifrost") {
    Geom = new BifrostGeometry(Stats, CaenConfiguration);
  } else if (settings.DetectorName == "miracles") {
    Geom = new MiraclesGeometry(Stats, CaenConfiguration);
  } else if (settings.DetectorName == "cspec") {
    Geom = new CspecGeometry(Stats, CaenConfiguration);
  } else if (settings.DetectorName == "tbl3he") {
    Geom = new Tbl3HeGeometry(Stats, CaenConfiguration);
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

  ESSHeaderParser.setMaxPulseTimeDiff(
      CaenConfiguration.CaenParms.MaxPulseTimeNS);
  ESSHeaderParser.Packet.Time.setMaxTOF(CaenConfiguration.CaenParms.MaxTOFNS);
}

CaenInstrument::~CaenInstrument() {}

void CaenInstrument::processReadouts() {
  XTRACE(DATA, DEB, "Reference time is %" PRIi64,
         ESSHeaderParser.Packet.Time.getRefTimeUInt64());
  /// \todo sometimes PrevPulseTime maybe?
  auto packet_ref_time =
      static_cast<int64_t>(ESSHeaderParser.Packet.Time.getRefTimeUInt64());
  for (auto &Serializer : Serializers)
    Serializer->checkAndSetReferenceTime(packet_ref_time);

  /// Traverse readouts, calculate pixels
  for (auto &Data : CaenParser.Result) {
    XTRACE(DATA, DEB, "Fiber %u, FEN %u", Data.FiberId, Data.FENId);
    bool validData = Geom->validateReadoutData(Data);
    if (not validData) {
      XTRACE(DATA, WAR, "Invalid Data, skipping readout");
      continue;
    }

    // Calculate TOF in ns
    uint64_t TimeOfFlight = ESSHeaderParser.Packet.Time.getTOF(
        ESSTime(Data.TimeHigh, Data.TimeLow));

    XTRACE(DATA, DEB,
           "PulseTime     %" PRIu64 ", Previous PulseTime: %" PRIu64
           ", Calculated ToF %" PRIu64 " ",
           ESSHeaderParser.Packet.Time.getRefTimeUInt64(),
           ESSHeaderParser.Packet.Time.getPrevRefTimeUInt64(), TimeOfFlight);

    if (TimeOfFlight == ESSHeaderParser.Packet.Time.InvalidTOF) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      counters.TimeError++;
      continue;
    }

    XTRACE(DATA, DEB,
           "  Data: time (%10u, %10u) tof %llu, SeqNo %u, Group %u, A %d, B "
           "%d, C %d, D %d",
           Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.Unused, Data.Group,
           Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

    // Calculate pixel using template wrapper with automatic error counting
    uint32_t PixelId = Geom->calcPixel(Data);

    // Determine the correct serializer for this pixel
    auto SerializerId = Geom->calcSerializer(Data);

    if (PixelId == 0) {
      XTRACE(DATA, DEB,
             "Pixel Error  Data: time (%10u, %10u) tof %llu, SeqNo %u, Group "
             "%u, A %u, B "
             "%u, C %u, D %u",
             Data.TimeHigh, Data.TimeLow, TimeOfFlight, Data.Unused, Data.Group,
             Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
      // PixelErrors are now counted automatically by DetectorGeometry::calcPixel()
    } else if (SerializerId >= Serializers.size()) {
      XTRACE(EVENT, WAR, "Serializer identification error");
      counters.SerializerErrors++;
    } else {
      XTRACE(EVENT, DEB, "Pixel %u, TOF %u", PixelId, TimeOfFlight);
      Serializers[SerializerId]->addEvent(static_cast<int32_t>(TimeOfFlight),
                                          static_cast<int32_t>(PixelId));
      counters.Events++;
    }

  } // for()
}

} // namespace Caen
