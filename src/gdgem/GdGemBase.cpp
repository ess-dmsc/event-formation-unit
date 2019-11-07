/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// plugin for gdgem detector data reception, parsing and event_ formation
///
//===----------------------------------------------------------------------===//

#include "GdGemBase.h"

#include <common/reduction/matching/GapMatcher.h>
#include <common/reduction/matching/CenterMatcher.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <gdgem/srs/BuilderVMM3.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/generators/BuilderReadouts.h>
#include <common/EV42Serializer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/Producer.h>
#include <efu/Server.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>

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
  if ((std::abs(calib.adc_offset) <= 1e-6) and (std::abs(calib.adc_slope) <= 1e-6) and (std::abs(calib.time_offset) <= 1e-6) and (std::abs(calib.time_slope) <= 1e-6)) {
    *obytes =
        snprintf(output, SERVER_BUFFER_SIZE, "<error> no calibration exist");
    return -Parser::EBADARGS;
  }

 *obytes = snprintf(output, SERVER_BUFFER_SIZE, "%s adc_offset: %f adc_slope: %f, time_offset: %f time_slope: %f",
                     cmd.c_str(), calib.adc_offset, calib.adc_slope, calib.time_offset, calib.time_slope);

  return Parser::OK;
}

GdGemBase::GdGemBase(BaseSettings const &settings, struct NMXSettings &LocalSettings) :
       Detector("NMX", settings), NMXSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  LOG(INIT, Sev::Info, "NMX Config file: {}", NMXSettings.ConfigFile);
  nmx_opts = Gem::NMXConfig(NMXSettings.ConfigFile, NMXSettings.CalibrationFile);

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", stats_.rx_packets);
  Stats.create("rx_bytes", stats_.rx_bytes);
  Stats.create("i2pfifo_dropped", stats_.fifo_push_errors);

  Stats.create("processing_idle", stats_.processing_idle);
  Stats.create("fifo_seq_errors", stats_.fifo_seq_errors);

  // Parser
  Stats.create("parser_frame_seq_errors", stats_.parser_frame_seq_errors);
  Stats.create("parser_frame_missing_errors", stats_.parser_frame_missing_errors);
  Stats.create("parser_framecounter_overflows", stats_.parser_framecounter_overflows);
  Stats.create("parser_timestamp_seq_errors", stats_.parser_timestamp_seq_errors);
  Stats.create("parser_timestamp_lost_errors", stats_.parser_timestamp_lost_errors);
  Stats.create("parser_timestamp_overflows", stats_.parser_timestamp_overflows);
  Stats.create("parser_good_frames", stats_.parser_good_frames);
  Stats.create("parser_bad_frames", stats_.parser_bad_frames);
  Stats.create("parser_error_bytes", stats_.parser_error_bytes);
  Stats.create("parser_markers", stats_.parser_markers);
  Stats.create("parser_data", stats_.parser_data);
  Stats.create("parser_total", stats_.parser_readouts);

  // Builder
  Stats.create("hits_bad_geometry", stats_.hits_bad_geometry);
  Stats.create("hits_bad_adc", stats_.hits_bad_adc);
  Stats.create("hits_good", stats_.hits_good);

  // Clustering
  Stats.create("clusters_total", stats_.clusters_total);
  Stats.create("clusters_x_only", stats_.clusters_x_only);
  Stats.create("clusters_y_only", stats_.clusters_y_only);
  Stats.create("clusters_xy", stats_.clusters_xy);

  // Event Analysis  
  Stats.create("events_good", stats_.events_good);
  Stats.create("events_bad", stats_.events_bad);
  Stats.create("events_filter_rejects", stats_.events_filter_rejects);
  Stats.create("events_geom_errors", stats_.events_geom_errors);
  Stats.create("events_good_hits", stats_.events_good_hits);

  Stats.create("tx_bytes", stats_.tx_bytes);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", stats_.kafka_produce_fails);
  Stats.create("kafka.ev_errors", stats_.kafka_ev_errors);
  Stats.create("kafka.ev_others", stats_.kafka_ev_others);
  Stats.create("kafka.dr_errors", stats_.kafka_dr_errors);
  Stats.create("kafka.dr_others", stats_.kafka_dr_noerrors);
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
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver nmxdata(local);

  nmxdata.setBufferSizes(0 /*use default */, EFUSettings.DetectorRxBufferSize);
  nmxdata.checkRxBufferSizes(EFUSettings.DetectorRxBufferSize);
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
      XTRACE(PROCESS, DEB, "rdsize: %zu", rdsize);
      stats_.rx_packets++;
      stats_.rx_bytes += rdsize;

      // stats_.fifo_free = input2proc_fifo.free();
      if (!input2proc_fifo.push(eth_index)) {
        stats_.fifo_push_errors++;
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
  auto sum = e.ClusterA.weight_sum() + e.ClusterB.weight_sum();
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
        nmx_opts.calfile, stats_, nmx_opts.enable_data_processing);

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

  if(nmx_opts.matcher_name == "CenterMatcher") {
    auto matcher = std::make_shared<CenterMatcher>(
        nmx_opts.time_config.acquisition_window()*5, 0, 1);
    matcher->set_max_delta_time(nmx_opts.matcher_max_delta_time);
    matcher->set_time_algorithm(nmx_opts.time_algorithm);
    matcher_ = matcher;
  }
  else {
    auto matcher = std::make_shared<GapMatcher>(
        nmx_opts.time_config.acquisition_window()*5, 0, 1);
    matcher->set_minimum_time_gap(nmx_opts.matcher_max_delta_time);
    matcher_ = matcher;
  }

  hists_.set_cluster_adc_downshift(nmx_opts.cluster_adc_downshift);

  sample_next_track_ = nmx_opts.send_tracks;
}

void GdGemBase::cluster_plane(HitVector &hits,
                              std::shared_ptr<AbstractClusterer> clusterer, bool flush) {
  sort_chronologically(hits);
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

  stats_.clusters_total  += matcher_->matched_events.size();
  for (auto& event : matcher_->matched_events)
  {
    if (!event.both_planes()) {
      if (event.ClusterA.hit_count() != 0) {
        stats_.clusters_x_only++;
      }
      else {
        stats_.clusters_y_only++;
      }
      continue;
    }

    stats_.clusters_xy++;

    neutron_event_ = nmx_opts.analyzer_->analyze(event);

    /// Sample only tracks that are good in both planes
    if (sample_next_track_
        && (event.total_hit_count() >= nmx_opts.track_sample_minhits))
    {
//      LOG(PROCESS, Sev::Debug, "Serializing track: {}", event.to_string(true));
      sample_next_track_ = !track_serializer.add_track(event,
                                                       neutron_event_.x.center,
                                                       neutron_event_.y.center);
    }

    if (!neutron_event_.good)
    {
      stats_.events_bad++;
      continue;
    }

    if (!nmx_opts.filter.valid(event))
    {
      stats_.events_filter_rejects++;
      continue;
    }

    // \todo this logic is a hack to accomodate MG
    if (nmx_opts.geometry.nz() > 1) {
      pixelid_ = nmx_opts.geometry.pixel3D(
          neutron_event_.x.center_rounded(),
          neutron_event_.y.center_rounded(),
          neutron_event_.z.center_rounded());
    } else {
      pixelid_ = nmx_opts.geometry.pixel2D(
          neutron_event_.x.center_rounded(), neutron_event_.y.center_rounded());
    }

    if (!nmx_opts.geometry.valid_id(pixelid_))
    {
      stats_.events_geom_errors++;
      continue;
    }

    // Histogram cluster ADC only for valid events
    if (nmx_opts.hit_histograms)
    {
      bin(hists_, event);
    }
    //not needed, gives wrong results, since time order of clusters
    //not guaranteed
    /*
    if (neutron_event_.time < previous_full_time_) {
      LOG(PROCESS, Sev::Error, "Event time sequence error: {} < {}",
             neutron_event_.time, previous_full_time_);
    }
    previous_full_time_ = neutron_event_.time;
    */

    truncated_time_ = neutron_event_.time - recent_pulse_time_;
    // \todo try different limits
    if (!have_pulse_time_ ||
        (truncated_time_ > max_pulse_window_ns)) {
      have_pulse_time_ = true;
      recent_pulse_time_ = neutron_event_.time;
      truncated_time_ = 0;
      if (event_serializer.eventCount())
        stats_.tx_bytes += event_serializer.produce();
      event_serializer.pulseTime(recent_pulse_time_);
//      LOG(PROCESS, Sev::Debug, "New offset time selected: {}", recent_pulse_time_);
    }

//    LOG(PROCESS, Sev::Debug, "Good event: time={}, pixel={} from {}",
//        truncated_time_, pixelid_, neutron_event_.to_string());

    stats_.tx_bytes += event_serializer.addEvent(
        static_cast<uint32_t>(truncated_time_), pixelid_);
    stats_.events_good++;
    stats_.events_good_hits += event.total_hit_count();
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

  auto ProduceEvents = [&event_producer](auto DataBuffer, auto Timestamp) {
    event_producer.produce(DataBuffer, Timestamp);
  };

  auto ProduceMonitor = [&monitor_producer](auto DataBuffer, auto Timestamp) {
    monitor_producer.produce(DataBuffer, Timestamp);
  };

  auto ProduceHits = [&hits_producer](auto DataBuffer, auto Timestamp) {
    hits_producer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer ev42serializer(kafka_buffer_size, "nmx", ProduceEvents);

  Gem::TrackSerializer track_serializer(256, "nmx_tracks");
  track_serializer.set_callback(ProduceMonitor);

  HistogramSerializer hist_serializer(hists_.needed_buffer_size(), "nmx");
  hist_serializer.set_callback(ProduceMonitor);

  Gem::TrackSerializer raw_serializer(1500, "nmx_hits");
  raw_serializer.set_callback(ProduceHits);

  TSCTimer global_time, report_timer;

  unsigned int data_index;
  int cnt = 0;
  std::chrono::time_point<std::chrono::system_clock> timeEnd, timeStart;
  timeStart = std::chrono::system_clock::now();
  double avg = 0;
  double total = 0;
  int rep = 0;
  while (true) {
    // stats_.fifo_free = input2proc_fifo.free();
    if (!input2proc_fifo.pop(data_index)) {
      stats_.processing_idle++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        stats_.fifo_seq_errors++;
      } else {
        builder_->process_buffer(
            eth_ringbuf->getDataBuffer(data_index), len);
        cnt++;
        if(cnt == 10000) {
          cnt = 0;
          timeEnd = std::chrono::system_clock::now();
          int ms = std::chrono::duration_cast < std::chrono::milliseconds > (timeEnd - timeStart).count();
          rep++;
          total += ms*0.001;
          avg = total/rep;
          std::cout << rep << ": 10000 x process_buffer " << ms*0.001 << " s, avg = " << avg << " s\n";
          timeStart = std::chrono::system_clock::now();
        }
        
        if (nmx_opts.enable_data_processing) {
          stats_.hits_good += (builder_->hit_buffer_x.size()
            + builder_->hit_buffer_y.size());
          if (nmx_opts.send_raw_hits) {
            Event dummy_event;
            for (const auto& e : builder_->hit_buffer_x) {
              dummy_event.ClusterA.insert(e);
            }
            for (const auto& e : builder_->hit_buffer_y) {
              dummy_event.ClusterB.insert(e);
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
          }
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

      stats_.tx_bytes += ev42serializer.produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      stats_.kafka_produce_fails = event_producer.stats.produce_fails;
      stats_.kafka_ev_errors = event_producer.stats.ev_errors;
      stats_.kafka_ev_others = event_producer.stats.ev_others;
      stats_.kafka_dr_errors = event_producer.stats.dr_errors;
      stats_.kafka_dr_noerrors = event_producer.stats.dr_noerrors;

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
