// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
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
#include <common/kafka/EV44Serializer.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <freia/Counters.h>
#include <freia/FreiaBase.h>
#include <freia/FreiaInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters &counters,
                                 BaseSettings &settings,
                                 EV44Serializer *serializer)
    : counters(counters), Settings(settings), Serializer(serializer) {

  loadConfigAndCalib();

  // We can now use the settings in Conf

  Geom.setGeometry(Conf.FileParameters.InstrumentGeometry);

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.FileParameters.MaxPulseTimeNS);
}

void FreiaInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());

  /// \brief Covers FREIA, ESTIA and AMOR
  /// Differentiation is done using the InstrumentGeometry value
  /// see usage in Config.cpp and Geometry.h
  Conf = Config("Freia", Settings.ConfigFile);
  Conf.loadAndApplyConfig();

  XTRACE(INIT, ALW, "Creating vector of %d builders (one per cassette/hybrid)",
         Conf.NumHybrids);
  builders = std::vector<EventBuilder2D>(Conf.NumHybrids);

  for (EventBuilder2D &builder : builders) {
    builder.matcher.setMaximumTimeGap(Conf.CfgParms.MaxMatchingTimeGap);
    builder.ClustererX.setMaximumTimeGap(Conf.CfgParms.MaxClusteringTimeGap);
    builder.ClustererY.setMaximumTimeGap(Conf.CfgParms.MaxClusteringTimeGap);
    if (Conf.CfgParms.SplitMultiEvents) {
      builder.matcher.setSplitMultiEvents(
          Conf.CfgParms.SplitMultiEvents,
          Conf.CfgParms.SplitMultiEventsCoefficientLow,
          Conf.CfgParms.SplitMultiEventsCoefficientHigh);
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
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->checkAndSetReferenceTime(
      /// \todo sometimes PrevPulseTime maybe?
      ESSReadoutParser.Packet.Time.getRefTimeUInt64());

  for (const auto &readout : VMMParser.Result) {

    XTRACE(DATA, INF,
           "readout: FiberId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.FiberId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    // Convert from physical rings to logical rings
    uint8_t Ring = readout.FiberId / 2;

    // Check for configuration mismatch
    if (Ring > VMM3Config::MaxRing) {
      counters.RingMappingErrors++;
      continue;
    }

    if (readout.FENId > VMM3Config::MaxFEN) {
      counters.FENMappingErrors++;
      continue;
    }

    uint8_t HybridId = readout.VMM >> 1;
    if (!Conf.getHybrid(Ring, readout.FENId, HybridId).Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             Ring, readout.FENId, HybridId);
      counters.HybridMappingErrors++;
      continue;
    }

    ESSReadout::Hybrid &Hybrid =
        Conf.getHybrid(Ring, readout.FENId, readout.VMM >> 1);

    uint8_t Asic = readout.VMM & 0x1;
    XTRACE(DATA, DEB, "Asic calculated to be %u", Asic);
    VMM3Calibration &Calib = Hybrid.VMMs[Asic];
    XTRACE(DATA, DEB, "Hybrid at: %p", &Hybrid);
    XTRACE(DATA, DEB, "Calibration at: %p", &Hybrid.VMMs[Asic]);

    // apply adc thresholds
    if (readout.OTADC < Hybrid.ADCThresholds[Asic][0]) {
      counters.ADCBelowThreshold++;
      continue;
    }

    uint64_t TimeNS =
        ESSReadout::ESSTime::toNS(readout.TimeHigh, readout.TimeLow).count();
    int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS,
           TDCCorr);

    TimeNS += TDCCorr;
    XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
    uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);

    XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC & 0x3FF,
           ADC);

    // If the corrected ADC reaches maximum value we count the occurance but
    // use the new value anyway
    if (ADC >= 1023) {
      counters.MaxADC++;
    }

    // Now we add readouts with the calibrated time and adc to the x,y builders
    if (Geom.isXCoord(readout.VMM)) {
      uint16_t XCoord = Geom.xCoord(Hybrid.XOffset, readout.VMM, readout.Channel);
      XTRACE(DATA, INF,
             "X: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
             TimeNS, PlaneX, XCoord, readout.Channel, ADC);

      if (XCoord == GeometryBase::InvalidCoord) {
        counters.InvalidXCoord++;
        continue;
      }

      builders[Hybrid.HybridNumber].insert({TimeNS, XCoord, ADC, PlaneX});

    } else { // implicit isYCoord
      uint16_t yCoord =
          Geom.yCoord(Hybrid.YOffset, readout.VMM, readout.Channel);
      XTRACE(DATA, INF,
             "Y: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
             TimeNS, PlaneY, yCoord, readout.Channel, ADC);
      if (yCoord == GeometryBase::InvalidCoord) {
        counters.InvalidYCoord++;
        continue;
      }

      builders[Hybrid.HybridNumber].insert({TimeNS, yCoord, ADC, PlaneY});
    }
  }

  for (auto &builder : builders) {
    builder.flush(true); // Do matching
  }
}

void FreiaInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSReferenceTime &TimeRef = ESSReadoutParser.Packet.Time;
  // XTRACE(EVENT, DEB, "Number of events: %u", Events.size());
  for (const auto &e : Events) {
    if (e.empty()) {
      XTRACE(EVENT, DEB, "Empty event");
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB,
             "\n================== NO COINCIDENCE =====================\n\n");
      XTRACE(EVENT, DEB, "Event has no coincidence\n %s\n",
             e.to_string({}, true).c_str());
      counters.EventsNoCoincidence++;

      if (not e.ClusterB.empty()) {
        counters.EventsMatchedWireOnly++;
      }

      if (not e.ClusterA.empty()) {
        counters.EventsMatchedStripOnly++;
      }
      continue;
    }

    // Discard if there are gaps in the strip or wire channels
    if (Conf.WireGapCheck) {
      if (e.ClusterB.hasGap(Conf.CfgParms.MaxGapWire)) {
        XTRACE(EVENT, DEB, "Event discarded due to wire gap");
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    if (Conf.StripGapCheck) {
      if (e.ClusterA.hasGap(Conf.CfgParms.MaxGapStrip)) {
        XTRACE(EVENT, DEB, "Event discarded due to strip gap");
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, INF, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.timeStart();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64, EventTime,
           TimeRef.getRefTimeUInt64());

    if (TimeRef.getRefTimeUInt64() > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF!");
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.getRefTimeUInt64();

    if (TimeOfFlight > Conf.FileParameters.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.FileParameters.MaxTOFNS);
      counters.MaxTOFErrors++;
      continue;
    }

    // calculate local x and y using center of mass
    auto x = static_cast<uint16_t>(std::round(e.ClusterA.coordCenter()));
    auto y = static_cast<uint16_t>(std::round(e.ClusterB.coordCenter()));
    auto PixelId = Geom.pixel2D(x, y);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u TOF: %u, x %u, y %u, pixel %u",
             time, TimeOfFlight, x, y, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, pixel %u", time,
           TimeOfFlight, x, y, PixelId);
    Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}

} // namespace Freia
