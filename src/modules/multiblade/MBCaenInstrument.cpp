// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Multigrid processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <multiblade/MBCaenInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

// Old PSI data (ESS mask) uses monitor channels
#define MB_MONITOR_CHANNEL

namespace Multiblade {

/// \brief load configuration and calibration files
MBCaenInstrument::MBCaenInstrument(struct Counters & counters,
    BaseSettings & EFUSettings,
    CAENSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {


    // Setup Instrument according to configuration file
    MultibladeConfig = Config(ModuleSettings.ConfigFile);

    if (!moduleSettings.FilePrefix.empty()) {
      dumpfile = ReadoutFile::create(moduleSettings.FilePrefix + "-" + timeString());
    }


    ncass = MultibladeConfig.getCassettes();
    nwires = MultibladeConfig.getWires();
    nstrips = MultibladeConfig.getStrips();
    mbgeom = MBGeometry(ncass, nwires, nstrips);

    if (MultibladeConfig.getInstrument() == Config::InstrumentGeometry::Estia) {
      XTRACE(PROCESS, ALW, "Setting instrument configuration to Estia");
      mbgeom.setConfigurationEstia();
      essgeom = ESSGeometry(ncass * nwires, nstrips, 1, 1);
      topic = "estia_detector";
      monitor = "estia_monitor";
    } else {
      mbgeom.setConfigurationFreia();
      XTRACE(PROCESS, ALW, "Setting instrument configuration to Freia");
      essgeom = ESSGeometry(nstrips, ncass * nwires, 1, 1);
      topic = "freia_detector";
      monitor = "freia_monitor";
    }

    if (MultibladeConfig.getDetectorType() == Config::DetectorType::MB18) {
      XTRACE(PROCESS, ALW, "Setting detector to MB18");
      mbgeom.setDetectorMB18();
    } else {
      XTRACE(PROCESS, ALW, "Setting detector to MB16");
      mbgeom.setDetectorMB16();
    }

    builders = std::vector<EventBuilder>(ncass);
    for (EventBuilder & builder : builders) {
      builder.setTimeBox(2010);
    }

    // Kafka producers and flatbuffer serialisers
    // Monitor producer
    Producer monitorprod(EFUSettings.KafkaBroker, monitor);
    auto ProduceHist = [&monitorprod](auto DataBuffer, auto Timestamp) {
      monitorprod.produce(DataBuffer, Timestamp);
    };
    histfb.set_callback(ProduceHist);
    histograms = Hists(std::max(ncass * nwires, ncass * nstrips), 65535);
    histfb = HistogramSerializer(histograms.needed_buffer_size(), "multiblade");
    //

}


// Moved from MBCaenBase to better support unit testing
bool MBCaenInstrument::parsePacket(char * data, int length,  EV42Serializer & ev42ser) {

  int res = parser.parse(data, length);

  counters.ReadoutsErrorBytes += parser.Stats.error_bytes;
  counters.ReadoutsErrorVersion += parser.Stats.error_version;
  counters.ReadoutsSeqErrors += parser.Stats.seq_errors;

  if (res < 0) {
    return false;
  }

  XTRACE(DATA, DEB, "Received %d readouts from digitizer %d",
         parser.MBHeader->numElements, parser.MBHeader->digitizerID);

  counters.ReadoutsCount += parser.MBHeader->numElements;

  uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
  ev42ser.pulseTime(efu_time);

  if (dumpfile) {
    dumpfile->push(parser.readouts);
  }

  auto cassette = MultibladeConfig.Mappings->cassette(parser.MBHeader->digitizerID);
  if (cassette < 0) {
    XTRACE(DATA, WAR, "Invalid digitizerId: %d", parser.MBHeader->digitizerID);
    counters.PacketBadDigitizer++;
    return false;
  }

  FixJumpsAndSort(cassette, parser.readouts);
  builders[cassette].flush();

  return true;
}


// New EF algorithm - Needed to sort readouts in time
bool compareByTime(const Readout &a, const Readout &b) {
  return a.local_time < b.local_time;
}

// New EF algorithm - buffers data according to time and sorts before
// processing
void MBCaenInstrument::FixJumpsAndSort(int cassette, std::vector<Readout> &vec) {
  int64_t Gap{43'000'000};
  int64_t PrevTime{0xffffffffff};
  std::vector<Readout> temp;

  for (auto &Readout : vec) {
    int64_t Time = (uint64_t)(Readout.local_time * MultibladeConfig.TimeTickNS);

    if ((PrevTime - Time) < Gap) {
      temp.push_back(Readout);
    } else {
      XTRACE(CLUSTER, DEB, "Wrap: %4d, Time: %lld, PrevTime: %lld, diff %lld",
             counters.ReadoutsTimerWraps, Time, PrevTime, (PrevTime - Time));
      counters.ReadoutsTimerWraps++;
      std::sort(temp.begin(), temp.end(), compareByTime);
      LoadAndProcessReadouts(cassette, temp);

      temp.clear();
      temp.push_back(Readout);
    }
    PrevTime = Time;
  }
  LoadAndProcessReadouts(cassette, temp);
}

//
void MBCaenInstrument::LoadAndProcessReadouts(int cassette, std::vector<Readout> &vec) {
  for (auto &dp : vec) {
    if (not mbgeom.isValidCh(dp.channel)) {
      counters.ReadoutsInvalidChannel++;
      continue;
    }

    if (dp.adc > MultibladeConfig.max_valid_adc) {
      counters.ReadoutsInvalidAdc++;
      continue;
    }

    uint8_t plane = mbgeom.getPlane(dp.channel);

    #ifdef MB_MONITOR_CHANNEL
    #define MONITOR_DIGITIZER 137
    #define MONITOR_CHANNEL 62
    #define MONITOR_PLANE 0
    if ((dp.digitizer == MONITOR_DIGITIZER) and
        (dp.channel == MONITOR_CHANNEL) and
        (plane == MONITOR_PLANE)) {
      counters.ReadoutsMonitor++;
      continue;
    }
    #endif

    uint16_t coord;
    if (plane == 0) {
      coord = mbgeom.getx(cassette, dp.channel);
    } else  if (plane == 1) {
      coord = mbgeom.gety(cassette, dp.channel);
    } else {
      counters.ReadoutsInvalidPlane++;
      continue;
    }

    counters.ReadoutsGood++;

    XTRACE(DATA, DEB, "time %u, channel %u, adc %u",
           dp.local_time, dp.channel, dp.adc);
    XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
           dp.debug().c_str(), cassette, plane, coord);

    assert(dp.local_time * MultibladeConfig.TimeTickNS < 0xffffffff);
    uint64_t Time = (uint64_t)(dp.local_time * MultibladeConfig.TimeTickNS);

    builders[cassette].insert({Time, coord, dp.adc, plane});
  }
  builders[cassette].flush();
}


} // namespace
