/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// plugin for gdgem detector data reception, parsing and event formation
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
#include <common/Timer.h>
#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>

// This is depending on the CPU, but we do not rely on this to be accurate
static constexpr int TscMHz {2900};

// Emulating ESS 14Hz pulse
static constexpr uint64_t MaxPulseWindowNs {1000000000 / 14};

int GdGemBase::getCalibration(std::vector<std::string> CmdArgs,
                        char *Output,
                        unsigned int *OutputBytes) {
  std::string Command = "NMX_GET_CALIB";
  LOG(CMD, Sev::Info, "{}", Command);
  if (CmdArgs.size() != 4) {
    LOG(CMD, Sev::Warning, "{}: wrong number of arguments", Command);
    return -Parser::EBADARGS;
  }

  int Fec = atoi(CmdArgs.at(1).c_str());
  int Asic = atoi(CmdArgs.at(2).c_str());
  int Channel = atoi(CmdArgs.at(3).c_str());
  auto Calib = NMXOpts.calfile->getCalibration(Fec, Asic, Channel);
  if ((std::abs(Calib.adc_offset) <= 1e-6) and
      (std::abs(Calib.adc_slope) <= 1e-6) and
      (std::abs(Calib.time_offset) <= 1e-6) and
      (std::abs(Calib.time_slope) <= 1e-6)) {
    *OutputBytes =
        snprintf(Output, SERVER_BUFFER_SIZE, "<error> no calibration exist");
    return -Parser::EBADARGS;
  }

 *OutputBytes = snprintf(Output, SERVER_BUFFER_SIZE, "%s adc_offset: %f adc_slope: %f, time_offset: %f time_slope: %f",
                     Command.c_str(), Calib.adc_offset, Calib.adc_slope, Calib.time_offset, Calib.time_slope);

  return Parser::OK;
}

GdGemBase::GdGemBase(BaseSettings const &Settings, struct NMXSettings &LocalSettings) :
       Detector("NMX", Settings), NMXSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  LOG(INIT, Sev::Info, "NMX Config file: {}", NMXSettings.ConfigFile);
  NMXOpts = Gem::NMXConfig(NMXSettings.ConfigFile, NMXSettings.CalibrationFile);

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", stats_.RxPackets);
  Stats.create("receive.bytes", stats_.RxBytes);
  Stats.create("receive.dropped", stats_.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", stats_.FifoSeqErrors);

  Stats.create("thread.processing_idle", stats_.ProcessingIdle);

  // Parser
  Stats.create("readouts.good_frames", stats_.ParserGoodFrames);
  Stats.create("readouts.bad_frames", stats_.ParserBadFrames);
  Stats.create("readouts.error_bytes", stats_.ParserErrorBytes);

  Stats.create("readouts.frame_seq_errors", stats_.ParserFrameSeqErrors);
  Stats.create("readouts.frame_missing_errors", stats_.ParserFrameMissingErrors);
  Stats.create("readouts.framecounter_overflows", stats_.ParserFramecounterOverflows);

  Stats.create("readouts.timestamp_seq_errors", stats_.ParserTimestampSeqErrors);
  Stats.create("readouts.timestamp_lost_errors", stats_.ParserTimestampLostErrors);
  Stats.create("readouts.timestamp_overflows", stats_.ParserTimestampOverflows);

  Stats.create("readouts.count", stats_.ParserReadouts);
  Stats.create("readouts.markers", stats_.ParserMarkers);
  Stats.create("readouts.data", stats_.ParserData);


  // Builder
  Stats.create("hits.good", stats_.HitsGood);
  Stats.create("hits.outside_roi", stats_.HitsOutsideRegion);
  Stats.create("hits.over_threshold", stats_.ParserOverThreshold);
  Stats.create("hits.bad_plane", stats_.HitsBadPlane);
  Stats.create("hits.bad_geometry", stats_.HitsBadGeometry);
  Stats.create("hits.bad_adc", stats_.HitsBadAdc);

  // Clustering
  Stats.create("clusters.total", stats_.ClustersTotal);
  Stats.create("clusters.x", stats_.ClustersXOnly);
  Stats.create("clusters.y", stats_.ClustersYOnly);
  Stats.create("clusters.x_and_y", stats_.ClustersXAndY);

  // Event Analysis
  Stats.create("events.good", stats_.EventsGood);
  Stats.create("events.bad", stats_.EventsBad);
  Stats.create("events.outside_region", stats_.EventsOutsideRegion);
  Stats.create("events.filter_rejects", stats_.EventsFilterRejects);
  Stats.create("events.geom_errors", stats_.EventsGeomErrors);
  Stats.create("events.good_hits", stats_.EventsGoodHits);

  Stats.create("transmit.bytes", stats_.TxBytes);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", stats_.KafkaProduceFails);
  Stats.create("kafka.ev_errors", stats_.KafkaEvErrors);
  Stats.create("kafka.ev_others", stats_.KafkaEvOthers);
  Stats.create("kafka.dr_errors", stats_.KafkaDrErrors);
  Stats.create("kafka.dr_others", stats_.KafkaDrNoErrors);
  // clang-format on

  if (!NMXSettings.FilePrefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
           NMXSettings.FilePrefix);

  std::function<void()> inputFunc = [this]() { GdGemBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() { GdGemBase::processingThread(); };
  Detector::AddThreadFunction(processingFunc, "processing");

  AddCommandFunction("NMX_GET_CALIB",
                     [this](std::vector<std::string> CmdArgs, char *Output,
                            unsigned int *OutputBytes) {
                       return GdGemBase::getCalibration(CmdArgs, Output, OutputBytes);
                     });
}

void GdGemBase::inputThread() {
  /** Connection setup */
  Socket::Endpoint LocalSocket(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver DataReceiver(LocalSocket);

  DataReceiver.setBufferSizes(0 /*use default */, EFUSettings.DetectorRxBufferSize);
  DataReceiver.checkRxBufferSizes(EFUSettings.DetectorRxBufferSize);
  DataReceiver.printBufferSizes();
  DataReceiver.setRecvTimeout(0, 100000); /// secs, usecs

  for (;;) {
    ssize_t ReadSize{0};
    unsigned int RxBufferIndex = RxRingbuffer.getDataIndex();

    /** this is the processing step */
    RxRingbuffer.setDataLength(RxBufferIndex,
                               0); /**\todo \todo buffer corruption can occur */
    if ((ReadSize = DataReceiver.receive(RxRingbuffer.getDataBuffer(RxBufferIndex),
                                  RxRingbuffer.getMaxBufSize())) > 0) {
      RxRingbuffer.setDataLength(RxBufferIndex, ReadSize);
      XTRACE(PROCESS, DEB, "rdsize: %zu", ReadSize);
      stats_.RxPackets++;
      stats_.RxBytes += ReadSize;

      // stats_.fifo_free = InputFifo.free();
      if (!InputFifo.push(RxBufferIndex)) {
        stats_.FifoPushErrors++;
      } else {
        RxRingbuffer.getNextBuffer();
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

void GdGemBase::applyConfiguration() {
  LOG(INIT, Sev::Info, "NMXConfig:\n{}", NMXOpts.debug());

  if (NMXOpts.builder_type == "VMM3") {
    builder_ = std::make_shared<Gem::BuilderVMM3>(
        NMXOpts.time_config, NMXOpts.srs_mappings,
        NMXOpts.adc_threshold,
        NMXSettings.FilePrefix,
        NMXSettings.PMin,
        NMXSettings.PMax,
        NMXSettings.PWidth,
        NMXOpts.calfile, stats_, NMXOpts.enable_data_processing);

  } else if (NMXOpts.builder_type == "Readouts") {
    builder_ = std::make_shared<Gem::BuilderReadouts>(
        NMXOpts.srs_mappings,
        NMXOpts.adc_threshold,
        NMXSettings.FilePrefix);

  } else if (NMXOpts.builder_type == "Hits") {
    builder_ = std::make_shared<Gem::BuilderHits>();
  } else {
    LOG(INIT, Sev::Error, "Unrecognized builder type in config");
  }

  clusterer_x_ = std::make_shared<GapClusterer>(
      NMXOpts.clusterer_x.max_time_gap, NMXOpts.clusterer_x.max_strip_gap);
  clusterer_y_ = std::make_shared<GapClusterer>(
      NMXOpts.clusterer_y.max_time_gap, NMXOpts.clusterer_y.max_strip_gap);

  if(NMXOpts.matcher_name == "CenterMatcher") {
    auto matcher = std::make_shared<CenterMatcher>(
        NMXOpts.time_config.acquisition_window()*5, 0, 1);
    matcher->set_max_delta_time(NMXOpts.matcher_max_delta_time);
    matcher->set_time_algorithm(NMXOpts.time_algorithm);
    matcher_ = matcher;
  }
  else {
    auto matcher = std::make_shared<GapMatcher>(
        NMXOpts.time_config.acquisition_window()*5, 0, 1);
    matcher->set_minimum_time_gap(NMXOpts.matcher_max_delta_time);
    matcher_ = matcher;
  }

  hists_.set_cluster_adc_downshift(NMXOpts.cluster_adc_downshift);

  sample_next_track_ = NMXOpts.send_tracks;
}

void GdGemBase::clusterPlane(HitVector &hits,
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

void GdGemBase::performClustering(bool flush) {
  // \todo we can parallelize this (per plane)

  if (builder_->hit_buffer_x.size()) {
    clusterPlane(builder_->hit_buffer_x, clusterer_x_, flush);
  }

  if (builder_->hit_buffer_y.size()) {
    clusterPlane(builder_->hit_buffer_y, clusterer_y_, flush);
  }

  // \todo but we cannot parallelize this, this is the critical path
  matcher_->match(flush);
}

void GdGemBase::processEvents(EV42Serializer& event_serializer,
                               Gem::TrackSerializer& track_serializer) {

  // This may be required if you start seeing "Event time sequence error" messages
//  std::sort(matcher_->matched_events.begin(), matcher_->matched_events.end(),
//            [](const Event &e1, const Event &e2) {
//              return e1.time_end() < e2.time_end();
//            });

  // \todo we can potentially infinitely parallelize this
  //       as each iteration is completely independent, other than
  //       everything going to the same serializers

  stats_.ClustersTotal  += matcher_->matched_events.size();
  for (auto& event : matcher_->matched_events)
  {
    if (!event.both_planes()) {
      if (event.ClusterA.hit_count() != 0) {
        stats_.ClustersXOnly++;
      }
      else {
        stats_.ClustersYOnly++;
      }
      continue;
    }

    stats_.ClustersXAndY++;

    neutron_event_ = NMXOpts.analyzer_->analyze(event);

    /// Sample only tracks that are good in both planes
    if (sample_next_track_
        && (event.total_hit_count() >= NMXOpts.track_sample_minhits))
    {
//      LOG(PROCESS, Sev::Debug, "Serializing track: {}", event.to_string(true));
      sample_next_track_ = !track_serializer.add_track(event,
                                                       neutron_event_.x.center,
                                                       neutron_event_.y.center);
    }

    if (!neutron_event_.good)
    {
      stats_.EventsBad++;
      continue;
    }

    if (!NMXOpts.filter.valid(event))
    {
      stats_.EventsFilterRejects++;
      continue;
    }

    // \todo this logic is a hack to accomodate MG
    if (NMXOpts.geometry.nz() > 1) {
      pixelid_ = NMXOpts.geometry.pixel3D(
          neutron_event_.x.center_rounded(),
          neutron_event_.y.center_rounded(),
          neutron_event_.z.center_rounded());
    } else {
      auto x = neutron_event_.x.center_rounded();
      if (( x >= NMXSettings.PMin ) and ( x <= NMXSettings.PMax)) {
        pixelid_ = NMXOpts.geometry.pixel2D(
            neutron_event_.x.center_rounded(), neutron_event_.y.center_rounded());
      } else {
        stats_.EventsOutsideRegion++;
        pixelid_ = 0;
      }
    }

    if (!NMXOpts.geometry.valid_id(pixelid_))
    {
      stats_.EventsGeomErrors++;
      continue;
    }

    // Histogram cluster ADC only for valid events
    if (NMXOpts.hit_histograms)
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
        (truncated_time_ > MaxPulseWindowNs)) {
      have_pulse_time_ = true;
      recent_pulse_time_ = neutron_event_.time;
      truncated_time_ = 0;
      if (event_serializer.eventCount())
        stats_.TxBytes += event_serializer.produce();
      event_serializer.pulseTime(recent_pulse_time_);
//      LOG(PROCESS, Sev::Debug, "New offset time selected: {}", recent_pulse_time_);
    }

//    LOG(PROCESS, Sev::Debug, "Good event: time={}, pixel={} from {}",
//        truncated_time_, pixelid_, neutron_event_.to_string());

    stats_.TxBytes += event_serializer.addEvent(
        static_cast<uint32_t>(truncated_time_), pixelid_);
    stats_.EventsGood++;
    stats_.EventsGoodHits += event.total_hit_count();
  }
  matcher_->matched_events.clear();
}


void GdGemBase::processingThread() {
  applyConfiguration();
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

  EV42Serializer ev42serializer(KafkaBufferSize, "nmx", ProduceEvents);

  Gem::TrackSerializer track_serializer(256, "nmx_tracks");
  track_serializer.set_callback(ProduceMonitor);

  HistogramSerializer hist_serializer(hists_.needed_buffer_size(), "nmx");
  hist_serializer.set_callback(ProduceMonitor);

  Gem::TrackSerializer raw_serializer(1500, "nmx_hits");
  raw_serializer.set_callback(ProduceHits);

  TSCTimer ReportTimer;
  unsigned int DataIndex;
  /* Time for performance measurement
  Timer duration;
  duration.now();
  int cnt = 0;
  double avg = 0;
  double total = 0;
  int rep = 0;
  */
  while (true) {
    // stats_.fifo_free = InputFifo.free();
    if (!InputFifo.pop(DataIndex)) {
      stats_.ProcessingIdle++;
      usleep(1);
    } else {
      auto Length = RxRingbuffer.getDataLength(DataIndex);
      if (Length == 0) {
        stats_.FifoSeqErrors++;
      } else {
        builder_->process_buffer(RxRingbuffer.getDataBuffer(DataIndex), Length);
        /* Performance measurement
        cnt++;
        if(cnt == 10000) {
          cnt = 0;
          uint64_t us = duration.timeus();
          rep++;
          total += us*0.000001;
          avg = total/rep;
          LOG(PROCESS, Sev::Debug, "10000 x process_buffer: last time={}, avg time={}",
            us*0.000001, avg);
          duration.now();
        }
        */

        if (NMXOpts.enable_data_processing) {
          stats_.HitsGood += (builder_->hit_buffer_x.size()
            + builder_->hit_buffer_y.size());
          if (NMXOpts.send_raw_hits) {
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

          if (NMXOpts.hit_histograms) {
            for (const auto& e : builder_->hit_buffer_x) {
              bin(hists_, e);
            }
            for (const auto& e : builder_->hit_buffer_y) {
              bin(hists_, e);
            }
          }

          if (NMXOpts.perform_clustering) {
            // do not flush
            performClustering(false);
            processEvents(ev42serializer, track_serializer);
          }
          builder_->hit_buffer_x.clear();
          builder_->hit_buffer_y.clear();
        }
      }
    }

    // Flush on interval or exit
    if ((not runThreads) || (ReportTimer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TscMHz)) {

      if (not runThreads && NMXOpts.perform_clustering) {
        // flush everything first
        performClustering(true);
        processEvents(ev42serializer, track_serializer);
      }

      sample_next_track_ = NMXOpts.send_tracks;

      stats_.TxBytes += ev42serializer.produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      stats_.KafkaProduceFails = event_producer.stats.produce_fails;
      stats_.KafkaEvErrors = event_producer.stats.ev_errors;
      stats_.KafkaEvOthers = event_producer.stats.ev_others;
      stats_.KafkaDrErrors = event_producer.stats.dr_errors;
      stats_.KafkaDrNoErrors = event_producer.stats.dr_noerrors;

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

      ReportTimer.now();
    }
  }
}
