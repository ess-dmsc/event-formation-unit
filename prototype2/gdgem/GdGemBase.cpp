/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// plugin for gdgem detector data reception, parsing and event_ formation
///
//===----------------------------------------------------------------------===//

#include "GdGemBase.h"

#include <common/clustering/GapMatcher.h>
#include <common/clustering/GapClusterer.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <gdgem/srs/BuilderVMM3.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/generators/BuilderReadouts.h>
#include <common/EV42Serializer.h>
#include <common/HistSerializer.h>
#include <common/Producer.h>
#include <efu/Server.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
//#undef TRC_MASK
//#define TRC_MASK 0

// \todo MJC's workstation - not reliable
static constexpr int TscMHz {2900};

// Emulating ESS 17Hz pulse
static constexpr uint64_t max_pulse_window_ns {1000000000 / 17};

int GdGemBase::getCalibration(std::vector<std::string> cmdargs,
                        char *output,
                        unsigned int *obytes) {
  std::string cmd = "NMX_GET_CALIB";
  LOG(CMD, Sev::Info, "{}", cmd);
  if (cmdargs.size() != 4) {
    LOG(CMD, Sev::Warning, "{}: wrong number of arguments", cmd);
    return -Parser::EBADARGS;
  }

  int fec = atoi(cmdargs.at(1).c_str());
  int asic = atoi(cmdargs.at(2).c_str());
  int channel = atoi(cmdargs.at(3).c_str());
  auto calib = nmx_opts.calfile->getCalibration(fec, asic, channel);
  if ((std::abs(calib.offset) <= 1e-6) and (std::abs(calib.slope) <= 1e-6)) {
    *obytes =
        snprintf(output, SERVER_BUFFER_SIZE, "<error> no calibration exist");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "%s offset: %f slope: %f",
                     cmd.c_str(), calib.offset, calib.slope);

  return Parser::OK;
}

GdGemBase::GdGemBase(BaseSettings const &settings, struct NMXSettings &LocalSettings) :
       Detector("NMX", settings), NMXSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  LOG(INIT, Sev::Info, "NMX Config file: {}", NMXSettings.ConfigFile);
  nmx_opts = Gem::NMXConfig(NMXSettings.ConfigFile, NMXSettings.CalibrationFile);

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", mystats.rx_packets);
  Stats.create("rx_bytes", mystats.rx_bytes);
  Stats.create("i2pfifo_dropped", mystats.fifo_push_errors);

  Stats.create("processing_idle", mystats.processing_idle);
  Stats.create("fifo_seq_errors", mystats.fifo_seq_errors);

  // Parser
  Stats.create("fc_seq_errors", mystats.fc_seq_errors);
  Stats.create("bad_frames", mystats.bad_frames);
  Stats.create("good_frames", mystats.good_frames);
  Stats.create("readouts_error_bytes", mystats.readouts_error_bytes);
  Stats.create("readouts_total", mystats.readouts_total);

  // Builder
  Stats.create("readouts_bad_geometry", mystats.readouts_bad_geometry);
  Stats.create("readouts_bad_adc", mystats.readouts_bad_adc);
  Stats.create("readouts_adc_zero", mystats.readouts_adc_zero);
  Stats.create("readouts_good", mystats.readouts_good);

  // Clustering
  Stats.create("clusters_total", mystats.clusters_total);
  Stats.create("clusters_x_only", mystats.clusters_x_only);
  Stats.create("clusters_y_only", mystats.clusters_y_only);
  Stats.create("clusters_xy", mystats.clusters_xy);

  Stats.create("events_bad_utpc", mystats.events_bad_utpc);
  Stats.create("events_filter_rejects", mystats.events_filter_rejects);
  Stats.create("events_geom_errors", mystats.events_geom_errors);
  Stats.create("events_good", mystats.events_good);
  Stats.create("readouts_in_good_events", mystats.readouts_in_good_events);

  Stats.create("tx_bytes", mystats.tx_bytes);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka.ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka.ev_others", mystats.kafka_ev_others);
  Stats.create("kafka.dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka.dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  if (!NMXSettings.fileprefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
           NMXSettings.fileprefix);

  std::function<void()> inputFunc = [this]() { GdGemBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() { GdGemBase::processing_thread(); };
  Detector::AddThreadFunction(processingFunc, "processing");

  AddCommandFunction("NMX_GET_CALIB",
                     [this](std::vector<std::string> cmdargs, char *output,
                            unsigned int *obytes) {
                       return GdGemBase::getCalibration(cmdargs, output, obytes);
                     });

//  LOG(INIT, Sev::Info, "Creating {} NMX Rx ringbuffers of size {}",
//         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(
      eth_buffer_max_entries + 11); /**< \todo testing workaround */
  assert(eth_ringbuf != 0);
}

void GdGemBase::input_thread() {
  /** Connection setup */
  int rxBuffer, txBuffer;
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver nmxdata(local);

  nmxdata.setBufferSizes(0 /*use default */, EFUSettings.DetectorRxBufferSize);
  nmxdata.getBufferSizes(txBuffer, rxBuffer);
  if (rxBuffer < EFUSettings.DetectorRxBufferSize) {
    LOG(INIT, Sev::Error, "Receive buffer sizes too small, wanted {}, got {}",
           EFUSettings.DetectorRxBufferSize, rxBuffer);
    return;
  }
  nmxdata.printBufferSizes();
  nmxdata.setRecvTimeout(0, 100000); /// secs, usecs

  TSCTimer report_timer;
  for (;;) {
    ssize_t rdsize {0};
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index,
                               0); /**\todo \todo buffer corruption can occur */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                  eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
//      LOG(INPUT, Sev::Debug, "rdsize: {}", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;

      // mystats.fifo_free = input2proc_fifo.free();
      if (!input2proc_fifo.push(eth_index)) {
        mystats.fifo_push_errors++;
      } else {
        eth_ringbuf->getNextBuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      LOG(INPUT, Sev::Info, "Stopping input thread.");
      return;
    }
  }
}

void bin(Hists& hists, const Event &e)
{
  auto sum = e.c1.weight_sum() + e.c2.weight_sum();
  hists.bincluster(static_cast<uint32_t>(sum));
}

void bin(Hists& hists, const Hit &e)
{
  if (e.plane == 0) {
    hists.bin_x(e.coordinate, e.weight);
  } else if (e.plane == 1) {
    hists.bin_y(e.coordinate, e.weight);
  }
}

void GdGemBase::apply_configuration() {
  LOG(INIT, Sev::Info, "NMXConfig:\n{}", nmx_opts.debug());

  if (nmx_opts.builder_type == "VMM3") {
    builder_ = std::make_shared<Gem::BuilderVMM3>(
        nmx_opts.time_config, nmx_opts.srs_mappings,
        nmx_opts.adc_threshold,
        NMXSettings.fileprefix,
        nmx_opts.calfile);

  } else if (nmx_opts.builder_type == "Readouts") {
    builder_ = std::make_shared<Gem::BuilderReadouts>(
        nmx_opts.srs_mappings,
        nmx_opts.adc_threshold,
        NMXSettings.fileprefix);

  } else if (nmx_opts.builder_type == "Hits") {
    builder_ = std::make_shared<Gem::BuilderHits>();
  } else {
    LOG(INIT, Sev::Error, "Unrecognized builder type in config");
  }

  clusterer_x_ = std::make_shared<GapClusterer>(
      nmx_opts.clusterer_x.max_time_gap, nmx_opts.clusterer_x.max_strip_gap);
  clusterer_y_ = std::make_shared<GapClusterer>(
      nmx_opts.clusterer_y.max_time_gap, nmx_opts.clusterer_y.max_strip_gap);

  matcher_ = std::make_shared<GapMatcher>(
      nmx_opts.time_config.acquisition_window()*5,
      nmx_opts.matcher_max_delta_time);

  hists_.set_cluster_adc_downshift(nmx_opts.cluster_adc_downshift);

  utpc_analyzer_ = std::make_shared<Gem::utpcAnalyzer>(
      nmx_opts.analyze_weighted,
      nmx_opts.analyze_max_timebins,
      nmx_opts.analyze_max_timedif);

  sample_next_track_ = nmx_opts.send_tracks;
}

void GdGemBase::cluster_plane(HitContainer &hits,
                              std::shared_ptr<AbstractClusterer> clusterer, bool flush) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });
  clusterer->cluster(hits);
  hits.clear();
  if (flush) {
    clusterer->flush();
  }
  if (!clusterer->clusters.empty()) {
//    LOG(PROCESS, Sev::Debug, "Adding {} clusters to matcher for plane {}",
//        clusterer->clusters.size(),
//        clusterer->clusters.front().plane());
    matcher_->insert(clusterer->clusters.front().plane(), clusterer->clusters);
  }
}

void GdGemBase::perform_clustering(bool flush) {
  // \todo we can parallelize this (per plane)

  if (builder_->hit_buffer_x.size()) {
    cluster_plane(builder_->hit_buffer_x, clusterer_x_, flush);
  }

  if (builder_->hit_buffer_y.size()) {
    cluster_plane(builder_->hit_buffer_y, clusterer_y_, flush);
  }

  // \todo but we cannot parallelize this, this is the critical path
  matcher_->match(flush);
}

void GdGemBase::process_events(EV42Serializer& event_serializer,
                               Gem::TrackSerializer& track_serializer) {

  // This may be required if you start seeing "Event time sequence error" messages
//  std::sort(matcher_->matched_events.begin(), matcher_->matched_events.end(),
//            [](const Event &e1, const Event &e2) {
//              return e1.time_end() < e2.time_end();
//            });

  // \todo we can potentially infinitely parallelize this
  //       as each iteration is completely independent, other than
  //       everything going to the same serializers

  mystats.clusters_total  += matcher_->matched_events.size();
  for (auto& event : matcher_->matched_events)
  {
    if (!event.both_planes()) {
      if (event.c1.hit_count() != 0) {
        mystats.clusters_x_only++;
      }
      else {
        mystats.clusters_y_only++;
      }
      continue;
    }

    mystats.clusters_xy++;

    utpc_ = utpc_analyzer_->analyze(event);

    /// Sample only tracks that are good in both planes
    if (sample_next_track_
        && (event.total_hit_count() >= nmx_opts.track_sample_minhits))
    {
//      LOG(PROCESS, Sev::Debug, "Serializing track: {}", event.debug(true));
      sample_next_track_ = !track_serializer.add_track(event,
                                                       utpc_.x.utpc_center,
                                                       utpc_.y.utpc_center);
    }

    if (!utpc_.good)
    {
      mystats.events_bad_utpc++;
      continue;
    }

    if (!nmx_opts.filter.valid(event, utpc_))
    {
      mystats.events_filter_rejects++;
      continue;
    }

    pixelid_ = nmx_opts.geometry.pixel2D(
        utpc_.x.utpc_center_rounded(), utpc_.y.utpc_center_rounded());

    if (!nmx_opts.geometry.valid_id(pixelid_))
    {
      mystats.events_geom_errors++;
      continue;
    }

    // Histogram cluster ADC only for valid events
    if (nmx_opts.hit_histograms)
    {
      bin(hists_, event);
    }

    if (utpc_.time < previous_full_time_) {
      LOG(PROCESS, Sev::Error, "Event time sequence error: {} < {}",
             utpc_.time, previous_full_time_);
    }
    previous_full_time_ = utpc_.time;

    truncated_time_ = utpc_.time - recent_pulse_time_;
    // \todo try different limits
    if (!have_pulse_time_ ||
        (truncated_time_ > max_pulse_window_ns)) {
      have_pulse_time_ = true;
      recent_pulse_time_ = utpc_.time;
      truncated_time_ = 0;
      if (event_serializer.eventCount())
        mystats.tx_bytes += event_serializer.produce();
      event_serializer.pulseTime(recent_pulse_time_);
//      LOG(PROCESS, Sev::Debug, "New offset time selected: {}", recent_pulse_time_);
    }

//    LOG(PROCESS, Sev::Debug, "Good event: time={}, pixel={} from {}",
//        truncated_time_, pixelid_, utpc_.debug());

    mystats.tx_bytes += event_serializer.addEvent(
        static_cast<uint32_t>(truncated_time_), pixelid_);
    mystats.events_good++;
    mystats.readouts_in_good_events += event.total_hit_count();
  }
  matcher_->matched_events.clear();
}


void GdGemBase::processing_thread() {
  apply_configuration();
  if (!builder_) {
    LOG(PROCESS, Sev::Error, "No builder specified, exiting thread");
    return;
    // \todo this only exits this thread, but EFU continues running
  }

  Producer event_producer(EFUSettings.KafkaBroker, "NMX_detector");
  Producer monitor_producer(EFUSettings.KafkaBroker, "NMX_monitor");
  Producer hits_producer(EFUSettings.KafkaBroker, "NMX_hits");

  EV42Serializer ev42serializer(kafka_buffer_size, "nmx");
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
  ev42serializer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &event_producer, std::placeholders::_1));

  Gem::TrackSerializer track_serializer(256, 1, "nmx_tracks");
  track_serializer.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitor_producer, std::placeholders::_1));

  HistSerializer hist_serializer(hists_.needed_buffer_size(), "nmx");
  hist_serializer.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitor_producer, std::placeholders::_1));

  Gem::TrackSerializer raw_serializer(1500, 1, "nmx_hits");
  raw_serializer.set_callback(
          std::bind(&Producer::produce2<uint8_t>, &hits_producer, std::placeholders::_1));
#pragma GCC diagnostic pop
  TSCTimer global_time, report_timer;

  unsigned int data_index;
  while (true) {
    // mystats.fifo_free = input2proc_fifo.free();
    if (!input2proc_fifo.pop(data_index)) {
      mystats.processing_idle++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        builder_->process_buffer(
            eth_ringbuf->getDataBuffer(data_index), len);

        // parser stats
        mystats.fc_seq_errors = builder_->stats.parser_fc_seq_errors;
        mystats.bad_frames = builder_->stats.parser_bad_frames;
        mystats.good_frames = builder_->stats.parser_good_frames;
        mystats.readouts_error_bytes = builder_->stats.parser_error_bytes;
        mystats.readouts_total = builder_->stats.parser_readouts;

        // builder stats
        mystats.readouts_bad_geometry = builder_->stats.geom_errors;
        mystats.readouts_bad_adc = builder_->stats.adc_rejects;
        mystats.readouts_adc_zero = builder_->stats.adc_zero;
        mystats.readouts_good += (builder_->hit_buffer_x.size()
            + builder_->hit_buffer_y.size());

        if (nmx_opts.send_raw_hits) {
          Event dummy_event;
          for (const auto& e : builder_->hit_buffer_x) {
            dummy_event.c1.insert(e);
          }
          for (const auto& e : builder_->hit_buffer_y) {
            dummy_event.c2.insert(e);
          }
          //LOG(PROCESS, Sev::Debug, "Sending raw data: {}", dummy_event.total_hit_count());
          raw_serializer.add_track(dummy_event, 0, 0);
        }

        if (nmx_opts.hit_histograms) {
          for (const auto& e : builder_->hit_buffer_x) {
            bin(hists_, e);
          }
          for (const auto& e : builder_->hit_buffer_y) {
            bin(hists_, e);
          }
        }

        if (nmx_opts.perform_clustering) {
          // do not flush
          perform_clustering(false);
          process_events(ev42serializer, track_serializer);
        } else {
          builder_->hit_buffer_x.clear();
          builder_->hit_buffer_y.clear();
        }
      }
    }

    // Flush on interval or exit
    if ((not runThreads) || (report_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TscMHz)) {

      if (not runThreads && nmx_opts.perform_clustering) {
        // flush everything first
        perform_clustering(true);
        process_events(ev42serializer, track_serializer);
      }

      sample_next_track_ = nmx_opts.send_tracks;

      mystats.tx_bytes += ev42serializer.produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      mystats.kafka_produce_fails = event_producer.stats.produce_fails;
      mystats.kafka_ev_errors = event_producer.stats.ev_errors;
      mystats.kafka_ev_others = event_producer.stats.ev_others;
      mystats.kafka_dr_errors = event_producer.stats.dr_errors;
      mystats.kafka_dr_noerrors = event_producer.stats.dr_noerrors;

      if (!hists_.isEmpty()) {
        LOG(PROCESS, Sev::Debug, "Sending histogram for {} readouts and {} clusters ",
               hists_.hit_count(), hists_.cluster_count());
        hist_serializer.produce(hists_);
        hists_.clear();
      }

      // checking for exit
      if (not runThreads) {
        LOG(PROCESS, Sev::Info, "Stopping processing thread.");
        // \todo why is this necessary? looks very wrong
        /// \todo this is a hack to force ~BuilderSRS() call
        builder_.reset();
        delete builder_.get(); /**< \todo see above */
        return;
      }

      report_timer.now();
    }
  }
}
