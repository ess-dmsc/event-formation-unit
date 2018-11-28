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
#include <gdgem/vmm3/BuilderVMM3.h>
#include <common/EV42Serializer.h>
#include <common/HistSerializer.h>
#include <common/Log.h>
#include <common/Producer.h>
#include <common/Trace.h>
#include <efu/Server.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

/** ----------------------------------------------------- */

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
  if ((abs(calib.offset) <= 1e-6) and (abs(calib.slope) <= 1e-6)) {
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
  Stats.setPrefix("efu.nmx");

  XTRACE(PROCESS, ALW, "NMX Config file: %s", NMXSettings.ConfigFile.c_str());
  nmx_opts = Gem::NMXConfig(NMXSettings.ConfigFile, NMXSettings.CalibrationFile);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", mystats.rx_packets);
  Stats.create("rx_bytes", mystats.rx_bytes);
  Stats.create("i2pfifo_dropped", mystats.fifo_push_errors);
  Stats.create("readouts", mystats.readouts);
  Stats.create("readouts_error_bytes", mystats.readouts_error_bytes);
  Stats.create("readouts_discarded", mystats.readouts_discarded);
  Stats.create("clusters_discarded", mystats.clusters_discarded);
  Stats.create("clusters_events", mystats.clusters_events);
  Stats.create("clusters_x", mystats.clusters_x);
  Stats.create("clusters_y", mystats.clusters_y);
  Stats.create("clusters_xy", mystats.clusters_xy);
  Stats.create("processing_idle", mystats.processing_idle);
  Stats.create("fifo_seq_errors", mystats.fifo_seq_errors);
  Stats.create("unclustered", mystats.unclustered);
  Stats.create("geom_errors", mystats.geom_errors);
  Stats.create("lost_frames", mystats.rx_seq_errors);
  Stats.create("bad_frames", mystats.bad_frames);
  Stats.create("good_frames", mystats.good_frames);
  Stats.create("tx_bytes", mystats.tx_bytes);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka_produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka_ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka_ev_others", mystats.kafka_ev_others);
  Stats.create("kafka_dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka_dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  if (!NMXSettings.fileprefix.empty())
    XTRACE(INIT, INF, "Dump h5 data in path: %s",
           NMXSettings.fileprefix.c_str());

  std::function<void()> inputFunc = [this]() { GdGemBase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() { GdGemBase::processing_thread(); };
  Detector::AddThreadFunction(processingFunc, "processing");

  AddCommandFunction("NMX_GET_CALIB",
                     [this](std::vector<std::string> cmdargs, char *output,
                            unsigned int *obytes) {
                       return GdGemBase::getCalibration(cmdargs, output, obytes);
                     });

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
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
    XTRACE(INIT, ERR, "Receive buffer sizes too small, wanted %d, got %d",
           EFUSettings.DetectorRxBufferSize, rxBuffer);
    return;
  }
  nmxdata.printBufferSizes();
  nmxdata.setRecvTimeout(0, 100000); /// secs, usecs

  TSCTimer report_timer;
  for (;;) {
    int rdsize;
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index,
                               0); /**\todo \todo buffer corruption can occur */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                  eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
      XTRACE(INPUT, DEB, "rdsize: %d", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;

      // mystats.fifo_free = input2proc_fifo.free();
      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo_push_errors++;
      } else {
        eth_ringbuf->getNextBuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void bin(Hists& hists, const Event &e)
{
  auto sum = e.c1.weight_sum() + e.c2.weight_sum();
  hists.bincluster(sum);
}

void bin(Hists& hists, const Hit &e)
{
  if (e.plane == 0) {
    hists.bin_x(e.coordinate, e.weight);
  } else if (e.plane == 1) {
    hists.bin_y(e.coordinate, e.weight);
  }
}

void bin_hists(Hists& hists, const std::list<Cluster>& cl)
{
  for (const auto& cluster : cl)
    for (const auto& e : cluster.hits)
      bin(hists, e);
}


void GdGemBase::apply_configuration() {
  XTRACE(INIT, ALW, "NMXConfig:\n%s", nmx_opts.debug().c_str());

  if (nmx_opts.builder_type == "VMM3") {
    XTRACE(INIT, DEB, "Using BuilderVMM3");
    builder_ = std::make_shared<Gem::BuilderVMM3>(
        nmx_opts.time_config, nmx_opts.srs_mappings,
        nmx_opts.clusterer_x.hit_adc_threshold,
        NMXSettings.fileprefix,
        nmx_opts.calfile);

  } else {
    XTRACE(INIT, ALW, "Unrecognized builder type in config");
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
  if (nmx_opts.hit_histograms) {
    bin_hists(hists_, clusterer->clusters);
  }
  if (!clusterer->empty()) {
    matcher_->insert(0, clusterer->clusters);
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

  // \todo we can potentially infinitely parallelize this
  //       as each iteration is completely independent, other than
  //       everything going to the same serializers

  for (auto& event : matcher_->matched_events) {
    XTRACE(PROCESS, DEB, "event_ready()");

    // mystats.unclustered = clusterer.unclustered();

    utpc_x_ = utpc_analyzer_->analyze(event.c1);
    utpc_y_ = utpc_analyzer_->analyze(event.c2);

    if (nmx_opts.hit_histograms) {
      bin(hists_, event);
    }

    if (event.both_planes()) {
      XTRACE(PROCESS, DEB, "event_.good");

      mystats.clusters_xy++;

      /// \todo Should it be here or outside of event_.valid()?
      if (sample_next_track_) {
        XTRACE(PROCESS, DEB, "Serializing track: %s\n", event.debug(true).c_str());
        sample_next_track_ = !track_serializer.add_track(event,
                                                        utpc_x_.utpc_center,
                                                        utpc_y_.utpc_center);
      }

      XTRACE(PROCESS, DEB, "x.center: %d, y.center %d",
             utpc_x_.utpc_center_rounded(),
             utpc_y_.utpc_center_rounded());

      if (nmx_opts.filter.valid(event, utpc_x_, utpc_y_)) {
        pixelid_ = nmx_opts.geometry.pixel2D(
            utpc_x_.utpc_center_rounded(), utpc_y_.utpc_center_rounded());

        if (!nmx_opts.geometry.valid_id(pixelid_)) {
          mystats.geom_errors++;
        } else {
          time_ = static_cast<uint32_t>(utpc_analyzer_->utpc_time(event.c1, event.c2));

          XTRACE(PROCESS, DEB, "time_: %d, pixelid_ %d", time_, pixelid_);

          mystats.tx_bytes += event_serializer.addEvent(time_, pixelid_);
          mystats.clusters_events++;
        }
      } else { // Does not meet criteria
        /** \todo increments counters when failing this */
      }
    } else { /// no valid event_
      if (event.c1.hit_count() != 0) {
        mystats.clusters_x++;
      } else {
        mystats.clusters_y++;
      }
      mystats.readouts_discarded += event.total_hit_count();
      mystats.clusters_discarded++;
    }
  }

  matcher_->matched_events.clear();
}


void GdGemBase::processing_thread() {
  apply_configuration();
  if (!builder_) {
    XTRACE(PROCESS, ERR, "No builder specified, exiting thread");
    return;
  }

  Producer event_producer(EFUSettings.KafkaBroker, "NMX_detector");
  Producer monitor_producer(EFUSettings.KafkaBroker, "NMX_monitor");

  EV42Serializer ev42serializer(kafka_buffer_size, "nmx");
  ev42serializer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &event_producer, std::placeholders::_1));

  Gem::TrackSerializer track_serializer(256, nmx_opts.track_sample_minhits,
                                        nmx_opts.time_config.target_resolution_ns());
  track_serializer.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitor_producer, std::placeholders::_1));

  HistSerializer hist_serializer(hists_.needed_buffer_size());
  hist_serializer.set_callback(
      std::bind(&Producer::produce2<uint8_t>, &monitor_producer, std::placeholders::_1));

  TSCTimer global_time, report_timer;


  unsigned int data_index;
  while (1) {
    // mystats.fifo_free = input2proc_fifo.free();
    if (!input2proc_fifo.pop(data_index)) {
      mystats.processing_idle++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        // printf("received packet with length %d\n", len);
        auto stats = builder_->process_buffer(
            eth_ringbuf->getDataBuffer(data_index), len);

        mystats.readouts += stats.valid_hits;
        mystats.readouts_error_bytes +=
            stats.error_bytes; // From srs data parser
        mystats.rx_seq_errors += stats.lost_frames;
        mystats.bad_frames += stats.bad_frames;
        mystats.good_frames += stats.good_frames;

        // do not flush
        perform_clustering(false);
        process_events(ev42serializer, track_serializer);
      }
    }

    // Flush on interval or exit
    if ((not runThreads) || (report_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ)) {

      if (not runThreads) {
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
        XTRACE(PROCESS, DEB, "Sending histogram for %zu hits and %zu clusters ",
               hists_.hit_count(), hists_.cluster_count());
        hist_serializer.produce(hists_);
        hists_.clear();
      }

      // checking for exit
      if (not runThreads) {
        XTRACE(INPUT, ALW, "Stopping processing thread.");
        /// \todo this is a hack to force ~BuilderSRS() call
        builder_.reset();
        delete builder_.get(); /**< \todo see above */
        return;
      }

      report_timer.now();
    }
  }
}

