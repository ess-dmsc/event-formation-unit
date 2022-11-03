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
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <ttlmonitor/TTLMonitorInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

/// \brief load configuration and calibration files
TTLMonitorInstrument::TTLMonitorInstrument(struct Counters &counters,
                                           BaseSettings &settings,
                                           std::vector<EV44Serializer> &serializers)

    : counters(counters), Settings(settings),
      Serializers(serializers) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config(Settings.ConfigFile);
  Conf.loadAndApply();

  if (!Settings.DumpFilePrefix.empty()) {
    std::string DumpFileName =
        Settings.DumpFilePrefix + "ttlmon_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);

  UseEveryNEvents = Settings.TTLMonitorReduceEvents;
}

void TTLMonitorInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are potentially now valid, negative TOF is not
  // possible, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here


  //TODO, have proper assertion heres
  //assert(Serializers != nullptr);
  for (EV44Serializer &Serializer : Serializers){
    counters.TxBytes += Serializer.checkAndSetReferenceTime(ESSReadoutParser.Packet.Time
                            .TimeInNS); /// \todo sometimes PrevPulseTime maybe?
  }

  XTRACE(DATA, DEB, "processMonitorReadouts()");
  for (const auto &readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB,
           "readout: RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    if (readout.RingId / 2 != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u for monitor readout", readout.RingId);
      counters.RingCfgErrors++;
      continue;
    }

    if (readout.FENId != Conf.Parms.MonitorFEN) {
      XTRACE(DATA, WAR, "Invalid FEN %d for monitor readout", readout.FENId);
      counters.FENCfgErrors++;
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

    if (readout.BC != 0) {
      XTRACE(DATA, WAR, "BC (%u) must be zero", readout.BC);
      counters.MonitorErrors++;
      continue;
    }

    if (readout.OTADC != 0) {
      XTRACE(DATA, WAR, "OTADC (%u) must be zero", readout.OTADC);
      counters.MonitorErrors++;
      continue;
    }

    if (readout.GEO != 0) {
      XTRACE(DATA, WAR, "GEO (%u) must be zero", readout.GEO);
      counters.MonitorErrors++;
      continue;
    }

    if (readout.TDC != 0) {
      XTRACE(DATA, WAR, "TDC (%u) must be zero", readout.TDC);
      counters.MonitorErrors++;
      continue;
    }

    if (readout.VMM != 0) {
      XTRACE(DATA, WAR, "VMM (%u) must be zero", readout.VMM);
      counters.MonitorErrors++;
      continue;
    }

    uint32_t PixelId = 1;
    XTRACE(DATA, DEB, "Pixel: %u TOF %" PRIu64 "", PixelId, TimeOfFlight);
    if (UseEveryNEvents == 1) {
      counters.TxBytes +=
          Serializers[readout.Channel].addEvent(TimeOfFlight, PixelId);
      counters.MonitorCounts++;
      UseEveryNEvents = Settings.TTLMonitorReduceEvents;
    } else {
      counters.MonitorIgnored++;
      UseEveryNEvents--;
    }
  }
}

/// \todo move into readout/vmm3 instead as this will be common
void TTLMonitorInstrument::dumpReadoutToFile(
    const ESSReadout::VMM3Parser::VMM3Data &Data) {
  VMM3::Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
  CurrentReadout.BC = Data.BC;
  CurrentReadout.OTADC = Data.OTADC;
  CurrentReadout.GEO = Data.GEO;
  CurrentReadout.TDC = Data.TDC;
  CurrentReadout.VMM = Data.VMM;
  CurrentReadout.Channel = Data.Channel;
  CurrentReadout.RingId = Data.RingId;
  CurrentReadout.FENId = Data.FENId;

  DumpFile->push(CurrentReadout);
}

} // namespace TTLMonitor
