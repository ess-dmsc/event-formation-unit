/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Processing pipeline for CSPEC instrument (Multi-Grid detector using
/// Mesytec readout)
///
//===----------------------------------------------------------------------===//

#include <multigrid/MultigridBase.h>

#include <common/Producer.h>
#include <common/TimeString.h>
#include <efu/Parser.h>
#include <efu/Server.h>

#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

//#include <multigrid/geometry/MG24Geometry.h>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#include "MultigridBase.h"
//#undef TRC_MASK
//#define TRC_MASK 0

// \todo MJC's workstation - not reliable
static constexpr int TscMHz{2900};

MultigridBase::MultigridBase(BaseSettings const &settings,
                             struct MultigridSettings &LocalSettings)
    : Detector("CSPEC", settings), ModuleSettings(LocalSettings) {
  Stats.setPrefix("efu.mgmesytec");

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", mystats.rx_packets);
  Stats.create("rx_bytes", mystats.rx_bytes);
  Stats.create("rx_discarded_bytes", mystats.discarded_bytes);
  Stats.create("triggers", mystats.triggers);
  Stats.create("bus_glitches", mystats.bus_glitches);
  Stats.create("bad_triggers", mystats.bad_triggers);
  Stats.create("readouts", mystats.readouts);
  Stats.create("readouts_discarded", mystats.readouts_discarded);
  Stats.create("readouts_culled", mystats.readouts_culled);
  Stats.create("geometry_errors", mystats.geometry_errors);
  Stats.create("timing_errors", mystats.timing_errors);
  Stats.create("events", mystats.events);
  Stats.create("tx_bytes", mystats.tx_bytes);
  // clang-format on

  LOG(INIT, Sev::Info, "Stream monitor data = {}",
      (ModuleSettings.monitor ? "YES" : "no"));
  if (!ModuleSettings.FilePrefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
        ModuleSettings.FilePrefix);

  std::function<void()> inputFunc = [this]() { MultigridBase::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");
}

void MultigridBase::init_config() {
  LOG(INIT, Sev::Info, "MG Config file: {}", ModuleSettings.ConfigFile);
  mg_config = Multigrid::Config(ModuleSettings.ConfigFile);
  LOG(INIT, Sev::Info, "Multigrid Config\n{}", mg_config.debug());

  if (ModuleSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);
}

void MultigridBase::process_events(EV42Serializer &ev42serializer) {
  std::sort(mg_config.reduction.matcher.matched_events.begin(),
            mg_config.reduction.matcher.matched_events.end(),
            [](const Event &e1,
               const Event &e2) {
              return e1.time_start() < e2.time_start();
            });

  for (auto &event : mg_config.reduction.matcher.matched_events) {

    if (event.plane1() == Multigrid::AbstractBuilder::external_trigger_plane) {
      if (HavePulseTime) {
        uint64_t PulsePeriod = event.time_start() - ev42serializer.pulseTime();
        ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
      }
      HavePulseTime = true;

      if (ev42serializer.eventCount())
        mystats.tx_bytes += ev42serializer.produce();
      ev42serializer.pulseTime(event.time_start());

//            XTRACE(PROCESS, DEB, "New pulse time: %u   shortest pulse period: %u",
//                   ev42serializer.pulseTime(), ShortestPulsePeriod);
    } else {
      auto neutron = mg_config.analyzer.analyze(event);
      if (!neutron.good)
        continue;
      //            XTRACE(PROCESS, DEB, "Neutron: %s ", neutron.debug().c_str());
      uint32_t pixel = mg_config.geometry.pixel3D(
          static_cast<uint32_t>(std::round(neutron.x)),
          static_cast<uint32_t>(std::round(neutron.y)),
          static_cast<uint32_t>(std::round(neutron.z))
      );
      auto time = static_cast<uint32_t>(neutron.time - ev42serializer.pulseTime());
//            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
      if (pixel == 0) {
        XTRACE(PROCESS, DEB, "Event geom error");
        mystats.geometry_errors++;
      } else if (neutron.time < ev42serializer.pulseTime()) {
        XTRACE(PROCESS, DEB, "Event before pulse");
        mystats.timing_errors++;
      } else if (time > (1.00004 * ShortestPulsePeriod)) {
        XTRACE(PROCESS, DEB, "Event out of pulse range");
        mystats.timing_errors++;
      } else {
//            mystats.readouts_culled += (parsed_readouts - mgEfu->used_readouts);
//              XTRACE(PROCESS, DEB, "Event good");
        mystats.events++;
        mystats.tx_bytes += ev42serializer.addEvent(time, pixel);
      }
    }
  }
  mg_config.reduction.matcher.matched_events.clear();
}

void MultigridBase::mainThread() {
  init_config();

  /** Connection setup */
  Socket::Endpoint
      local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  XTRACE(INIT, DEB, "server: %s, port %d", EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer ev42serializer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  ev42serializer.setProducerCallback(std::bind(&Producer::produce2<uint8_t>, &EventProducer, std::placeholders::_1));

  ev42serializer.pulseTime(0);

  uint8_t buffer[eth_buffer_size];
  ssize_t ReadSize{0};
  TSCTimer report_timer;
  while (true) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
//      XTRACE(PROCESS, DEB, "Processed UDP packet of size: %d", ReadSize);

      mg_config.builder->parse(Buffer<uint8_t>(buffer, static_cast<size_t>(ReadSize)));

      mystats.discarded_bytes = mg_config.builder->stats_discarded_bytes;
      mystats.triggers = mg_config.builder->stats_trigger_count;
      // \todo more stats

      if (!mg_config.builder->ConvertedData.empty()) {
        mystats.readouts += mg_config.builder->ConvertedData.size();

        for (const auto& hit : mg_config.builder->ConvertedData) {
          if (monitor.readouts)
            monitor.readouts->addEntry(hit.plane + 1, hit.coordinate, hit.time, hit.weight);
          if (monitor.hists) {
            if (hit.plane == 0)
              monitor.hists->bin_x(hit.coordinate, hit.weight);
            else if (hit.plane == 1)
              monitor.hists->bin_y(hit.coordinate, hit.weight);
          }
        }

        mg_config.reduction.ingest(mg_config.builder->ConvertedData);
        mg_config.reduction.perform_clustering(false);
        // \todo more stats

        process_events(ev42serializer);
      }
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TscMHz) {
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      LOG(PROCESS, Sev::Info, "Stopping processing thread.");
      // flush anything that remains
      mg_config.reduction.perform_clustering(true);
      process_events(ev42serializer);
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      return;
    }
  }
}
