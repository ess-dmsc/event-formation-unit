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

MultigridBase::MultigridBase(BaseSettings const &settings, MultigridSettings const &LocalSettings)
    : Detector("CSPEC", settings), ModuleSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", mystats.rx_packets);
  Stats.create("rx_bytes", mystats.rx_bytes);
  Stats.create("readouts_total", mystats.readouts_total);
  Stats.create("rx_discarded_bytes", mystats.parser_discarded_bytes);
  Stats.create("parser_triggers", mystats.parser_triggers);
  Stats.create("builder_glitch_rejects", mystats.builder_glitch_rejects);
  Stats.create("builder_filter_rejects", mystats.builder_filter_rejects);
  Stats.create("builder_geometry_errors", mystats.builder_geometry_errors);
  Stats.create("hits_total", mystats.hits_total);
  Stats.create("hits_time_seq_err", mystats.hits_time_seq_err);
  Stats.create("hits_bad_plane", mystats.hits_bad_plane);
  Stats.create("hits_used", mystats.hits_used);
  Stats.create("wire_clusters", mystats.wire_clusters);
  Stats.create("grid_clusters", mystats.grid_clusters);
  Stats.create("events_total", mystats.events_total);
  Stats.create("events_multiplicity_rejects", mystats.events_multiplicity_rejects);
  Stats.create("events_bad", mystats.events_bad);
  Stats.create("events_geometry_err", mystats.events_geometry_err);
  Stats.create("events_time_err", mystats.events_time_err);
  Stats.create("tx_events", mystats.tx_events);
  Stats.create("tx_bytes", mystats.tx_bytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails",             mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors",                 mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others",                 mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors",                 mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others",                 mystats.kafka_dr_noerrors);
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
      mystats.pulses++;

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
      mystats.events_total++;

      if ((event.c1.hit_count() > mg_config.max_wire_hits) ||
          (event.c2.hit_count() > mg_config.max_grid_hits))
      {
        mystats.events_multiplicity_rejects++;
        continue;
      }

      auto neutron = mg_config.analyzer.analyze(event);
      mystats.hits_used = mg_config.analyzer.stats_used_hits;

      if (!neutron.good) {
        mystats.events_bad++;
        continue;
      }
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
        mystats.events_geometry_err++;
      } else if (!HavePulseTime || (neutron.time < ev42serializer.pulseTime())) {
        XTRACE(PROCESS, DEB, "Event before pulse");
        mystats.events_time_err++;
      } else if (time > (1.00004 * ShortestPulsePeriod)) {
        XTRACE(PROCESS, DEB, "Event out of pulse range");
        mystats.events_time_err++;
      } else {
//              XTRACE(PROCESS, DEB, "Event good");
        mystats.tx_events++;
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
  Producer event_producer(EFUSettings.KafkaBroker, "C-SPEC_detector");

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  ev42serializer.setProducerCallback(std::bind(&Producer::produce2<uint8_t>, &event_producer, std::placeholders::_1));
#pragma GCC diagnostic pop
  
  ev42serializer.pulseTime(0);

  uint8_t buffer[eth_buffer_size];
  TSCTimer report_timer;
  while (true) {
    ssize_t ReadSize{0};
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
//      XTRACE(PROCESS, DEB, "Processed UDP packet of size: %d", ReadSize);

      mg_config.builder->parse(Buffer<uint8_t>(buffer, static_cast<size_t>(ReadSize)));

      mystats.readouts_total = mg_config.builder->stats_readouts_total;
      mystats.parser_discarded_bytes = mg_config.builder->stats_discarded_bytes;
      mystats.parser_triggers = mg_config.builder->stats_trigger_count;
      mystats.builder_glitch_rejects = mg_config.builder->stats_bus_glitch_rejects;
      mystats.builder_filter_rejects = mg_config.builder->stats_readout_filter_rejects;
      mystats.builder_geometry_errors = mg_config.builder->stats_digital_geom_errors;

      if (!mg_config.builder->ConvertedData.empty()) {
        mystats.hits_total += mg_config.builder->ConvertedData.size();

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
        mystats.hits_time_seq_err = mg_config.reduction.stats_time_seq_errors;
        mystats.hits_bad_plane = mg_config.reduction.stats_invalid_planes;
        mystats.wire_clusters = mg_config.reduction.stats_wire_clusters;
        mystats.grid_clusters = mg_config.reduction.stats_grid_clusters;
        process_events(ev42serializer);
      }
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TscMHz) {
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      mystats.kafka_produce_fails = event_producer.stats.produce_fails;
      mystats.kafka_ev_errors = event_producer.stats.ev_errors;
      mystats.kafka_ev_others = event_producer.stats.ev_others;
      mystats.kafka_dr_errors = event_producer.stats.dr_errors;
      mystats.kafka_dr_noerrors = event_producer.stats.dr_noerrors;

      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      LOG(PROCESS, Sev::Info, "Stopping processing thread.");
      // flush anything that remains

      mg_config.reduction.perform_clustering(true);
      mystats.hits_time_seq_err = mg_config.reduction.stats_time_seq_errors;
      mystats.hits_bad_plane = mg_config.reduction.stats_invalid_planes;
      mystats.wire_clusters = mg_config.reduction.stats_wire_clusters;
      mystats.grid_clusters = mg_config.reduction.stats_grid_clusters;
      process_events(ev42serializer);

      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      return;
    }
  }
}
