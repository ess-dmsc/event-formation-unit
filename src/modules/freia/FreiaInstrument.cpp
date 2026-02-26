// Copyright (C) 2021 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/geometry/vmm3/VMM3Geometry.h>
#include <common/kafka/EV44Serializer.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <freia/Counters.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

using namespace vmm3;
using namespace geometry;

namespace Freia {

FreiaInstrument::FreiaInstrument(struct Counters &counters,
                                 BaseSettings &settings,
                                 EV44Serializer &serializer,
                                 ESSReadout::Parser &essHeaderParser,
                                 Statistics &Stats,
                                 const DetectorType &detectorType)
    : counters(counters), Settings(settings), Serializer(serializer),
      ESSHeaderParser(essHeaderParser) {

  loadConfigAndCalib();
  // Geometry depends on DetectorType and configuration InstrumentGeometry
  // string
  Geom = createGeometry(detectorType, Conf.FileParms.InstrumentGeometry, Stats);
  ESSHeaderParser.setMaxPulseTimeDiff(Conf.FileParms.MaxPulseTimeNS);
  ESSHeaderParser.Packet.Time.setMaxTOF(Conf.FileParms.MaxTOFNS);
}

void FreiaInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());

  /// \brief Covers FREIA, ESTIA, AMOR, TBLMB via InstrumentGeometry value
  Conf = Config("Freia", Settings.ConfigFile);
  Conf.loadAndApplyConfig();

  XTRACE(INIT, ALW, "Creating vector of %d builders (one per cassette/hybrid)",
         Conf.NumHybrids);
  builders = std::vector<EventBuilder2D>(Conf.NumHybrids);

  for (EventBuilder2D &builder : builders) {
    builder.matcher.setMaximumTimeGap(Conf.MBFileParms.MaxMatchingTimeGap);
    builder.ClustererX.setMaximumTimeGap(Conf.MBFileParms.MaxClusteringTimeGap);
    builder.ClustererY.setMaximumTimeGap(Conf.MBFileParms.MaxClusteringTimeGap);
    if (Conf.MBFileParms.SplitMultiEvents) {
      builder.matcher.setSplitMultiEvents(
          Conf.MBFileParms.SplitMultiEvents,
          Conf.MBFileParms.SplitMultiEventsCoefficientLow,
          Conf.MBFileParms.SplitMultiEventsCoefficientHigh);
    }
  }

  if (Settings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file");
    Conf.loadAndApplyCalibration(Settings.CalibFile);
  }
}

void FreiaInstrument::processReadouts() {
  XTRACE(DATA, DEB,
         "\n================== NEW PACKET =====================\n\n");

  Serializer.checkAndSetReferenceTime(
      ESSHeaderParser.Packet.Time.getRefTimeUInt64());

  for (const auto &readout : VMMParser.Result) {
    XTRACE(DATA, INF,
           "readout: FiberId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.FiberId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    uint8_t Ring =
        DetectorGeometry<vmm3::VMM3Parser::VMM3Data>::calcRing(readout.FiberId);

    // Validate readout data (Ring, FEN, Hybrid)
    if (not Geom->validateReadoutData(readout)) {
      continue;
    }

    uint8_t HybridId = VMM3Geometry::calcHybridId(readout.VMM);

    // Validation of hybrid done in validateReadoutData()
    // Get hybrid mapping
    const Hybrid &Hybrid = Conf.getHybrid(Ring, readout.FENId, HybridId);

    const uint8_t Asic = readout.VMM & 0x1;
    XTRACE(DATA, DEB, "Asic calculated to be %u", Asic);

    const VMM3Calibration &Calib = Hybrid.VMMs[Asic];
    XTRACE(DATA, DEB, "Hybrid at: %p", &Hybrid);
    XTRACE(DATA, DEB, "Calibration at: %p", &Hybrid.VMMs[Asic]);

    // ADC threshold
    if (readout.OTADC < Hybrid.ADCThresholds[Asic][0]) {
      counters.ADCBelowThreshold++;
      continue;
    }

    uint64_t TimeNS =
        ESSReadout::ESSTime::toNS(readout.TimeHigh, readout.TimeLow).count();
    const auto TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS,
           TDCCorr);
    TimeNS += TDCCorr;

    // Extract raw 10-bit ADC value, then apply calibration
    const uint16_t RawADC = VMM3Geometry::getRawADC(readout.OTADC);
    const uint16_t ADC = Calib.ADCCorr(readout.Channel, RawADC);
    if (ADC >= Calib.VMM_ADC_10BIT_LIMIT) {
      counters.MaxADC++;
    }
    XTRACE(DATA, DEB, "ADC calibration from %u to %u", RawADC, ADC);

    // Use unified coordinate calculation with auto-detection
    auto result = Geom->calculateCoordinate(Hybrid.XOffset, Hybrid.YOffset,
                                            readout.VMM, readout.Channel);

    XTRACE(DATA, DEB, "%s: TimeNS %" PRIu64 ", Coord %u, Channel %u, ADC %u",
           result.isXPlane ? "X" : "Y", TimeNS, result.coord, readout.Channel,
           ADC);

    /// skip further processing if coordinate is invalid
    if (result.coord == VMM3Geometry::InvalidCoord) {
      continue;
    }

    // Insert into appropriate builder based on plane
    uint8_t plane = result.isXPlane ? PlaneX : PlaneY;
    builders[Hybrid.HybridNumber].insert({TimeNS, result.coord, ADC, plane});
  }

  for (auto &builder : builders) {
    builder.flush(true);
  }
}

void FreiaInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSReferenceTime &TimeRef = ESSHeaderParser.Packet.Time;
  // XTRACE(EVENT, DEB, "Number of events: %u", Events.size());
  for (const auto &Event : Events) {
    if (Event.empty()) {
      XTRACE(EVENT, DEB, "Empty event");
      continue;
    }

    if (not Event.both_planes()) {
      XTRACE(EVENT, DEB,
             "\n================== NO COINCIDENCE =====================\n\n");
      XTRACE(EVENT, DEB, "Event has no coincidence\n %s\n",
             Event.to_string({}, true).c_str());
      counters.EventsNoCoincidence++;

      if (not Event.ClusterB.empty()) {
        counters.EventsMatchedWireOnly++;
      }

      if (not Event.ClusterA.empty()) {
        counters.EventsMatchedStripOnly++;
      }
      continue;
    }

    // Discard if there are gaps in the strip or wire channels
    if (Conf.WireGapCheck) {
      if (Event.ClusterB.hasGap(Conf.MBFileParms.MaxGapWire)) {
        XTRACE(EVENT, DEB, "Event discarded due to wire gap");
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    if (Conf.StripGapCheck) {
      if (Event.ClusterA.hasGap(Conf.MBFileParms.MaxGapStrip)) {
        XTRACE(EVENT, DEB, "Event discarded due to strip gap");
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, INF, "Event Valid\n %s", Event.to_string({}, true).c_str());

    // Calculate TOF in ns
    auto EventTimeNs = esstime::TimeDurationNano(Event.timeStart());

    auto TimeOfFlight = TimeRef.getTOF(ESSReadout::ESSTime(EventTimeNs));

    if (not TimeOfFlight.has_value()) {
      XTRACE(DATA, WAR, "No valid TOF from PulseTime or PrevPulseTime");
      continue;
    }

    auto PixelId = Geom->calcPixel(Event);
    XTRACE(EVENT, DEB, "Calculated pixel ID: %u", PixelId);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR,
             "Bad pixel!: EventTime: %" PRIu64 " TOF: %" PRIu64
             ", pixel %" PRIu32 "",
             static_cast<uint64_t>(EventTimeNs.count()), TimeOfFlight.value(),
             PixelId);
      continue;
    }

    XTRACE(EVENT, INF,
           "EventTime: %" PRIu64 " TOF: %" PRIu64 ", pixel %" PRIu32 "",
           static_cast<uint64_t>(EventTimeNs.count()), TimeOfFlight.value(),
           PixelId);

    Serializer.addEvent(TimeOfFlight.value(), PixelId);
    counters.Events++;
  }

  Events.clear(); // else events will accumulate
}

} // namespace Freia
