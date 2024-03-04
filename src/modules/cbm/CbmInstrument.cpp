// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM is a dedicated module for TTL triggered beam monitor
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/time/TimeString.h>
#include <cbm/CbmInstrument.h>
#include <cbm/geometry/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(struct Counters &counters,
                                           BaseSettings &settings)

    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config(Settings.ConfigFile);
  Conf.loadAndApply();

  // if (!Settings.DumpFilePrefix.empty()) {
  //   std::string DumpFileName =
  //       Settings.DumpFilePrefix + "ttlmon_" + timeString();
  //   XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
  //   DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  // }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
}

void CbmInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (EV44Serializer *Serializer : SerializersPtr) {
    counters.TxBytes += Serializer->checkAndSetReferenceTime(
        ESSReadoutParser.Packet.Time.TimeInNS);
    /// \todo sometimes PrevPulseTime maybe?
  }

  XTRACE(DATA, DEB, "processMonitorReadouts() - has %zu entries",
         CbmParser.Result.size());
  for (const auto &readout : CbmParser.Result) {

    // if (DumpFile) {
    //   dumpReadoutToFile(readout);
    // }

    XTRACE(
        DATA, DEB,
        "readout: FiberId %d, FENId %d, POS %d, Channel %d, ADC %d, TimeLow %d",
        readout.FiberId, readout.FENId, readout.Pos, readout.Channel,
        readout.ADC, readout.TimeLow);

    int Ring = readout.FiberId / 2;
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid lring %u (expect %u) for monitor readout",
             Ring, Conf.Parms.MonitorRing);
      counters.RingCfgErrors++;
      continue;
    }

    if (readout.FENId != Conf.Parms.MonitorFEN) {
      XTRACE(DATA, WAR, "Invalid FEN %d for monitor readout", readout.FENId);
      counters.FENCfgErrors++;
      continue;
    }



    if (readout.Channel < Conf.Parms.MonitorOffset) {
      XTRACE(DATA, WAR, "Invalid Channel %d", readout.Channel);
      counters.ChannelCfgErrors++;
      continue;
    }

    // channel corrected for configurable channel offset
    int Channel = readout.Channel - Conf.Parms.MonitorOffset;
    if (Channel >= Conf.Parms.NumberOfMonitors) {
      XTRACE(DATA, WAR, "Invalid Channel %d (max is %d)", readout.Channel,
             Conf.Parms.NumberOfMonitors - 1 + Conf.Parms.MonitorOffset);
      counters.ChannelCfgErrors++;
      continue;
    }

    uint64_t TimeNS =
        ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    XTRACE(DATA, DEB, "TimeRef PrevTime %" PRIi64 "", TimeRef.PrevTimeInNS);
    XTRACE(DATA, DEB, "TimeRef CurrTime %" PRIi64 "", TimeRef.TimeInNS);
    XTRACE(DATA, DEB, "Time of readout  %" PRIi64 "", TimeNS);

    uint64_t TimeOfFlight = 0;
    if (TimeRef.TimeInNS > TimeNS) {
      TimeOfFlight = TimeNS - TimeRef.PrevTimeInNS;
    } else {
      TimeOfFlight = TimeNS - TimeRef.TimeInNS;
    }

    if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    uint32_t PixelId = 1;
    XTRACE(DATA, DEB, "Pixel: %u TOF %" PRIu64 "ns", PixelId, TimeOfFlight);
    counters.TxBytes +=
        SerializersPtr[Channel]->addEvent(TimeOfFlight, PixelId);
    counters.MonitorCounts++;
  }
}

} // namespace cbm
