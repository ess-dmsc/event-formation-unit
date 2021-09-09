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
#include <assert.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters & counters,
    //BaseSettings & EFUSettings,
    FreiaSettings &moduleSettings,
    EV42Serializer * serializer)
      : counters(counters)
      , ModuleSettings(moduleSettings)
      , Serializer(serializer) {

    XTRACE(INIT, ALW, "Loading configuration file %s",
           ModuleSettings.ConfigFile.c_str());
    Conf = Config(ModuleSettings.ConfigFile);

    if (!ModuleSettings.FilePrefix.empty()) {
      std::string DumpFileName = ModuleSettings.FilePrefix + "freia_" + timeString();
      XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
      DumpFile = VMM3::ReadoutFile::create(DumpFileName);
    }

    ESSReadoutParser.setMaxPulseTimeDiff(Conf.MaxPulseTimeNS);

    builder.setTimeBox(2010); // Time boxing

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
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel, readout.TimeLow);
    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId/2;

    if (Ring >= Conf.NumRings - 1) {
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


    uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    uint16_t ADC = readout.OTADC & 0x3FF;
    uint8_t Plane = (readout.VMM & 0x1) ^ 0x1;
    uint8_t Cassette = 1 + Conf.FENOffset[Ring] * Conf.CassettesPerFEN +
      FreiaGeom.cassette(readout.FENId, readout.VMM); // local cassette

    if (Plane == FreiaGeom.PlaneX) {
      XTRACE(DATA, DEB, "TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u",
         TimeNS, FreiaGeom.PlaneX, FreiaGeom.xCoord(readout.VMM, readout.Channel), readout.Channel);
      builder.insert({TimeNS, FreiaGeom.xCoord(readout.VMM, readout.Channel),
                      ADC, FreiaGeom.PlaneX});
    } else {
      XTRACE(DATA, DEB, "TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u",
         TimeNS, FreiaGeom.PlaneY, FreiaGeom.yCoord(Cassette, readout.VMM, readout.Channel), readout.Channel);
      builder.insert({TimeNS, FreiaGeom.yCoord(Cassette, readout.VMM, readout.Channel),
                      ADC, FreiaGeom.PlaneY});
    }
  }

  builder.flush(); // Do matching
}


void FreiaInstrument::generateEvents(void) {
  for (const auto &e : builder.Events) {

    if (!e.both_planes()) {
      counters.EventsNoCoincidence++;
      continue;
    }

    bool DiscardGap{true};
    // Discard if there are gaps in the strip channels
    if (DiscardGap) {
      if (e.ClusterB.hits.size() < e.ClusterB.coord_span()) {
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    // Discard if there are gaps in the wire channels
    if (DiscardGap) {
      if (e.ClusterA.hits.size() < e.ClusterA.coord_span()) {
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;

    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());
    // calculate local x and y using center of mass
    auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
    auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));

    // \todo implement this
    auto time = 0;
    auto pixel_id = essgeom.pixel2D(x, y);
    XTRACE(EVENT, INF, "time: %u, x %u, y %u, pixel %u", time, x, y, pixel_id);

    if (pixel_id == 0) {
      counters.PixelErrors++;
    } else {
      counters.TxBytes += Serializer->addEvent(time, pixel_id);
      counters.Events++;
    }
  }
  builder.Events.clear(); // else events will accumulate
}


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
