// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM is a dedicated module for TTL triggered beam monitor
///
//===----------------------------------------------------------------------===//

#include <CbmTypes.h>
#include <assert.h>
#include <cbm/CbmInstrument.h>
#include <cbm/geometry/Parser.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/ess/Parser.h>
#include <common/time/TimeString.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(struct Counters &counters, BaseSettings &settings)

    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config(Settings.ConfigFile);
  Conf.loadAndApply();

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

    XTRACE(DATA, DEB,
           "readout: FiberId %d, FENId %d, POS %d, Type %d, Channel %d, ADC "
           "%d, TimeLow %d",
           readout.FiberId, readout.FENId, readout.Pos, readout.Type,
           readout.Channel, readout.ADC, readout.TimeLow);

    int Ring = readout.FiberId / 2;
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u (expect %u) for monitor readout", Ring,
             Conf.Parms.MonitorRing);
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

    CbmType type = static_cast<CbmType>(readout.Type);

    uint32_t PixelId = 1;
    if (type == CbmType::IBM) {
      PixelId = readout.NPos & 0xFFFFFF; // Extract lower 24 bits
    }

    XTRACE(DATA, DEB, "CbmType: %s Pixel: %" PRIu32 " TOF %" PRIu64 "ns",
           type.to_string(), PixelId, TimeOfFlight);
    counters.TxBytes +=
        SerializersPtr[Channel]->addEvent(TimeOfFlight, PixelId);
    counters.MonitorCounts++;
  }
}

} // namespace cbm
