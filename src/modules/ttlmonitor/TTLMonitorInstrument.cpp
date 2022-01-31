// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor is a dedicated module for TTL triggered beam monitor
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <ttlmonitor/TTLMonitorInstrument.h>
#include <common/readout/vmm3/CalibFile.h>
#include <common/readout/vmm3/Readout.h>
#include <assert.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

/// \brief load configuration and calibration files
TTLMonitorInstrument::TTLMonitorInstrument(struct Counters & counters,
  TTLMonitorSettings &moduleSettings,
  EV42Serializer * serializer)
    : counters(counters)
    , ModuleSettings(moduleSettings)
    , Serializer(serializer) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  Conf = Config(ModuleSettings.ConfigFile);
  Conf.loadAndApply();

  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName = ModuleSettings.FilePrefix + "ttlmon_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
}



void TTLMonitorInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSTime & TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are potentially now valid, negative TOF is not
  // possible, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processMonitorReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "readout: RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel, readout.TimeLow);

    if (readout.RingId/2 != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u for monitor readout",
             readout.RingId);
      counters.RingCfgErrors++;
      continue;
    }

    if (readout.FENId != Conf.Parms.MonitorFEN) {
      XTRACE(DATA, WAR, "Invalid FEN %d for monitor readout",
             readout.FENId);
      counters.FENCfgErrors++;
      continue;
    }

    uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
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

    XTRACE(DATA, DEB, "TOF %" PRIu64 "", TimeOfFlight);
    // TTL monitor emits pixels == Channel + 1
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, readout.Channel + 1);
    counters.MonitorCounts++;
  }
}


/// \todo move into readout/vmm3 instead as this will be common
void TTLMonitorInstrument::dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data) {
  VMM3::Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
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


} // namespace
