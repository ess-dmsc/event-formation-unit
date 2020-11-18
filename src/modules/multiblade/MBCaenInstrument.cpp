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
#include <multiblade/MBCaenInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

/// \brief load configuration and calibration files
MBCaenInstrument::MBCaenInstrument(struct Counters & counters,
    BaseSettings & EFUSettings,
    CAENSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {


    // Setup Instrument according to configuration file
    MultibladeConfig = Config(ModuleSettings.ConfigFile);
    assert(MultibladeConfig.getDigitizers() != nullptr);

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
      topic = "ESTIA_detector";
      monitor = "ESTIA_monitor";
    } else {
      mbgeom.setConfigurationFreia();
      XTRACE(PROCESS, ALW, "Setting instrument configuration to Freia");
      essgeom = ESSGeometry(nstrips, ncass * nwires, 1, 1);
      topic = "FREIA_detector";
      monitor = "FREIA_monitor";
    }

    if (MultibladeConfig.getDetectorType() == Config::DetectorType::MB18) {
      XTRACE(PROCESS, ALW, "Setting detector to MB18");
      mbgeom.setDetectorMB18();
    } else {
      XTRACE(PROCESS, ALW, "Setting detector to MB16");
      mbgeom.setDetectorMB16();
    }

    builders = std::vector<EventBuilder>(ncass);

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
  if (parser.parse(data, length) < 0) {
    counters.ReadoutsErrorBytes += parser.Stats.error_bytes;
    counters.ReadoutsErrorVersion += parser.Stats.error_version;
    return false;
  }
  counters.ReadoutsSeqErrors += parser.Stats.seq_errors;

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
    XTRACE(DATA, WAR, "Invalid digitizerId: %d",
           parser.MBHeader->digitizerID);
    counters.PacketBadDigitizer++;
    return false;
  }

  for (const auto &dp : parser.readouts) {
    ingestOneReadout(cassette, dp);
  }
  builders[cassette].flush();

  return true;
}


void MBCaenInstrument::ingestOneReadout(int cassette, const Readout & dp) {

  if (not mbgeom.isValidCh(dp.channel)) {
    counters.ReadoutsInvalidChannel++;
    return;
  }

  if (dp.adc > MultibladeConfig.max_valid_adc) {
    counters.ReadoutsInvalidAdc++;
    return;
  }

  uint8_t plane = mbgeom.getPlane(dp.channel);
  uint16_t global_ch = mbgeom.getGlobalChannel(cassette, dp.channel);
  uint16_t coord;
  if (plane == 0) {
    if (global_ch == 30) {
      counters.ReadoutsMonitor++;
      return;
    }
    coord = mbgeom.getx(cassette, dp.channel);
    histograms.bin_x(global_ch, dp.adc);
  } else  if (plane == 1) {
    coord = mbgeom.gety(cassette, dp.channel);
    histograms.bin_y(global_ch, dp.adc);
  } else {
    counters.ReadoutsInvalidPlane++;
    return;
  }

  counters.ReadoutsGood++;

  XTRACE(DATA, DEB, "time %lu, channel %u, adc %u", dp.local_time, dp.channel, dp.adc);

  builders[cassette].insert({dp.local_time, coord, dp.adc, plane});

  XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
         dp.debug().c_str(), cassette, plane, coord);
  }


bool MBCaenInstrument::filterEvent(const Event & e) {
    if (!e.both_planes()) {
      XTRACE(EVENT, INF, "Event No Coincidence %s", e.to_string({}, true).c_str());
      counters.EventsNoCoincidence++;
      return true;
    }

    // \todo parametrize maximum time span - in opts?
    if (MultibladeConfig.filter_time_span && (e.time_span() > MultibladeConfig.filter_time_span_value)) {
      XTRACE(EVENT, INF, "Event filter time_span %s", e.to_string({}, true).c_str());
      counters.FiltersMaxTimeSpan++;
      return true;
    }

    if ((e.ClusterA.coord_span() > e.ClusterA.hit_count()) && (e.ClusterB.coord_span() > e.ClusterB.hit_count())) {
      XTRACE(EVENT, INF, "Event Chs not adjacent %s", e.to_string({}, true).c_str());
      counters.EventsNotAdjacent++;
      return true;
    }
    return false;
  }

} // namespace
