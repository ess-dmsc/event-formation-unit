// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Multigrid processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <freia/FreiaInstrument.h>
#include <readout/vmm3/Readout.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters & counters,
    //BaseSettings & EFUSettings,
    FreiaSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {

    XTRACE(INIT, ALW, "Loading configuration file %s",
           ModuleSettings.ConfigFile.c_str());
    Conf = Config(ModuleSettings.ConfigFile);

    if (!ModuleSettings.FilePrefix.empty()) {
      std::string DumpFileName = ModuleSettings.FilePrefix + "freia_" + timeString();
      XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
      DumpFile = VMM3::ReadoutFile::create(DumpFileName);
    }


    // builders = std::vector<EventBuilder>(ncass);
    // for (EventBuilder & builder : builders) {
    //   builder.setTimeBox(2010);
    // }

    // Kafka producers and flatbuffer serialisers
    // Monitor producer
    // Producer monitorprod(EFUSettings.KafkaBroker, monitor);
    // auto ProduceHist = [&monitorprod](auto DataBuffer, auto Timestamp) {
    //   monitorprod.produce(DataBuffer, Timestamp);
    // };
    // histfb.set_callback(ProduceHist);
    // histograms = Hists(std::max(ncass * nwires, ncass * nstrips), 65535);
    // histfb = HistogramSerializer(histograms.needed_buffer_size(), "freia");
    //
}


void FreiaInstrument::processReadouts(void) {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "RingId %d, FENId %d, VMM %d",
           readout.RingId, readout.FENId, readout.VMM);
    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId/2;

    if (Ring >= Conf.NumRings) {
      XTRACE(DATA, WAR, "Invalid RingId %d (physical %d) - max is %d logical",
             Ring, readout.RingId, Conf.NumRings - 1);
      counters.RingErrors++;
      continue;
    }

    if (readout.FENId > Conf.NumFens[Ring]) {
      XTRACE(DATA, WAR, "Invalid FEN %d (max is %d)",
             readout.FENId, Conf.NumFens[Ring]);
      counters.FENErrors++;
      continue;
    }

  }
}
// New EF algorithm - Needed to sort readouts in time
// bool compareByTime(const Readout &a, const Readout &b) {
//   return a.local_time < b.local_time;
// }

// New EF algorithm - buffers data according to time and sorts before
// processing
// void FreiaInstrument::FixJumpsAndSort(int __attribute__((unused)) cassette, __attribute__((unused)) std::vector<Readout> &vec) {
  // int64_t Gap{43'000'000};
  // int64_t PrevTime{0xffffffffff};
  // std::vector<Readout> temp;
  //
  // for (auto &Readout : vec) {
  //   int64_t Time = (uint64_t)(Readout.local_time * FreiaConfig.TimeTickNS);
  //
  //   if ((PrevTime - Time) < Gap) {
  //     temp.push_back(Readout);
  //   } else {
  //     XTRACE(CLUSTER, DEB, "Wrap: %4d, Time: %lld, PrevTime: %lld, diff %lld",
  //            counters.ReadoutsTimerWraps, Time, PrevTime, (PrevTime - Time));
  //     counters.ReadoutsTimerWraps++;
  //     std::sort(temp.begin(), temp.end(), compareByTime);
  //     LoadAndProcessReadouts(cassette, temp);
  //
  //     temp.clear();
  //     temp.push_back(Readout);
  //   }
  //   PrevTime = Time;
  // }
  // LoadAndProcessReadouts(cassette, temp);
// }

//
// void FreiaInstrument::LoadAndProcessReadouts(int __attribute__((unused)) cassette, __attribute__((unused)) std::vector<Readout> &vec) {
  // for (auto &dp : vec) {
  //   if (not mbgeom.isValidCh(dp.channel)) {
  //     counters.ReadoutsInvalidChannel++;
  //     continue;
  //   }
  //
  //   if (dp.adc > FreiaConfig.max_valid_adc) {
  //     counters.ReadoutsInvalidAdc++;
  //     continue;
  //   }
  //
  //   uint8_t plane = mbgeom.getPlane(dp.channel);
  //
  //   #ifdef MB_MONITOR_CHANNEL
  //   #define MONITOR_DIGITIZER 137
  //   #define MONITOR_CHANNEL 62
  //   #define MONITOR_PLANE 0
  //   if ((dp.digitizer == MONITOR_DIGITIZER) and
  //       (dp.channel == MONITOR_CHANNEL) and
  //       (plane == MONITOR_PLANE)) {
  //     counters.ReadoutsMonitor++;
  //     continue;
  //   }
  //   #endif
  //
  //   uint16_t coord;
  //   if (plane == 0) {
  //     coord = mbgeom.getx(cassette, dp.channel);
  //   } else  if (plane == 1) {
  //     coord = mbgeom.gety(cassette, dp.channel);
  //   } else {
  //     counters.ReadoutsInvalidPlane++;
  //     continue;
  //   }
  //
  //   counters.ReadoutsGood++;
  //
  //   XTRACE(DATA, DEB, "time %u, channel %u, adc %u",
  //          dp.local_time, dp.channel, dp.adc);
  //   XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
  //          dp.debug().c_str(), cassette, plane, coord);
  //
  //   assert(dp.local_time * FreiaConfig.TimeTickNS < 0xffffffff);
  //   uint64_t Time = (uint64_t)(dp.local_time * FreiaConfig.TimeTickNS);
  //
  //   builders[cassette].insert({Time, coord, dp.adc, plane});
  // }
  // builders[cassette].flush();
// }

/// \todo move into readout/vmm3 instead as this will be common
void FreiaInstrument::dumpReadoutToFile(const VMM3Parser::VMM3Data & Data) {
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
