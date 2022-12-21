// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor is a dedicated module for TTL triggered beam monitor
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/time/TimeString.h>
#include <ttlmonitor/geometry/Parser.h>
#include <ttlmonitor/TTLMonitorInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

/// \brief load configuration and calibration files
TTLMonitorInstrument::TTLMonitorInstrument(
    struct Counters &counters, BaseSettings &settings)

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

void TTLMonitorInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (EV44Serializer *Serializer : Serializers) {
    counters.TxBytes += Serializer->checkAndSetReferenceTime(
      ESSReadoutParser.Packet.Time.TimeInNS);
      /// \todo sometimes PrevPulseTime maybe?
  }

  XTRACE(DATA, DEB, "processMonitorReadouts() - has %zu entries",
         TTLMonParser.Result.size());
  for (const auto &readout : TTLMonParser.Result) {

    // if (DumpFile) {
    //   dumpReadoutToFile(readout);
    // }

    XTRACE(DATA, DEB,
           "readout: PRingId %d, FENId %d, POS %d, Channel %d, ADC %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.Pos, readout.Channel,
           readout.ADC, readout.TimeLow);

    int LRingId = readout.RingId/2;
    if (LRingId != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid lring %u (expect %u) for monitor readout",
        LRingId, Conf.Parms.MonitorRing);
      counters.RingCfgErrors++;
      continue;
    }

    if (readout.FENId != Conf.Parms.MonitorFEN) {
      XTRACE(DATA, WAR, "Invalid FEN %d for monitor readout", readout.FENId);
      counters.FENCfgErrors++;
      continue;
    }

    if (readout.Channel >= Conf.Parms.NumberOfMonitors) {
      XTRACE(DATA, WAR, "Invalid Channel %d (max is %d)",
             readout.Channel, Conf.Parms.NumberOfMonitors - 1);
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
         Serializers[readout.Channel]->addEvent(TimeOfFlight, PixelId);
      counters.MonitorCounts++;

  }
}

// /// \todo move into readout/vmm3 instead as this will be common
// void TTLMonitorInstrument::dumpReadoutToFile(
//     const ESSReadout::VMM3Parser::VMM3Data &Data) {
//   VMM3::Readout CurrentReadout;
//   CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
//   CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
//   CurrentReadout.PrevPulseTimeHigh =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
//   CurrentReadout.PrevPulseTimeLow =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
//   CurrentReadout.EventTimeHigh = Data.TimeHigh;
//   CurrentReadout.EventTimeLow = Data.TimeLow;
//   CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
//   CurrentReadout.BC = Data.BC;
//   CurrentReadout.OTADC = Data.OTADC;
//   CurrentReadout.GEO = Data.GEO;
//   CurrentReadout.TDC = Data.TDC;
//   CurrentReadout.VMM = Data.VMM;
//   CurrentReadout.Channel = Data.Channel;
//   CurrentReadout.PRingId = Data.PRingId;
//   CurrentReadout.FENId = Data.FENId;
//
//   DumpFile->push(CurrentReadout);
// }

} // namespace TTLMonitor
