/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <dataformats/multigrid/inc/json.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>

#include <gdgem/NMXConfig.h>
#include <common/HistSerializer.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <gdgem/generators/BuilderAPV.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/vmm2/BuilderVMM2.h>
#include <gdgem/vmm3/BuilderVMM3.h>

#include <gdgem/clustering/ClusterMatcher.h>
#include <gdgem/clustering/DoroClusterer.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <unistd.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

struct NMXSettingsStruct {
  std::string ConfigFile;
  int SRSParserID; //
} NMXSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) &parser) {
  parser.add_option("-f,--file", NMXSettings.ConfigFile,
                    "NMX (gdgem) specific config file")
      ->group("NMX")->required();
}

class NMX : public Detector {
public:
  NMX(BaseSettings settings);
  ~NMX();
  void input_thread();
  void processing_thread();

  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 2000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 12400;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  // Careful also using this for other NMX pipeline

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    // int64_t fifo_free;
    int64_t pad_a[5]; /**< @todo check alignment*/

    // Processing Counters
    int64_t readouts;
    int64_t readouts_discarded;
    int64_t readouts_error_bytes;
    int64_t processing_idle;
    int64_t unclustered;
    int64_t geom_errors;
    int64_t clusters_x;
    int64_t clusters_y;
    int64_t clusters_xy;
    int64_t clusters_events;
    int64_t clusters_discarded;
    int64_t tx_bytes;
    int64_t fifo_seq_errors;
    int64_t lost_frames;
    int64_t bad_frames;
    int64_t good_frames;
  } ALIGN(64) mystats;

  NMXConfig nmx_opts;

  std::shared_ptr<AbstractBuilder> builder_{nullptr};
  void init_builder();
};

PopulateCLIParser PopulateParser{SetCLIArguments};

NMX::~NMX() { printf("NMX detector destructor called\n"); }

NMX::NMX(BaseSettings settings) : Detector("NMX", settings) {
  Stats.setPrefix("efu.nmx");

  XTRACE(INIT, ALW, "Adding stats\n");
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
  Stats.create("lost_frames", mystats.lost_frames);
  Stats.create("bad_frames", mystats.bad_frames);
  Stats.create("good_frames", mystats.good_frames);
  Stats.create("tx_bytes", mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { NMX::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() { NMX::processing_thread(); };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(
      eth_buffer_max_entries + 11); /**< @todo testing workaround */
  assert(eth_ringbuf != 0);
}

const char *NMX::detectorname() { return classname; }

void NMX::input_thread() {
  /** Connection setup */
  // Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver nmxdata(local);
  // nmxdata.buflen(opts->buflen);
  nmxdata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  nmxdata.printBufferSizes();
  nmxdata.setRecvTimeout(0, 100000); /// secs, usecs

  int rdsize;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(
        eth_index, 0); /**@todo @fixme buffer corruption can occur */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                  eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
      XTRACE(INPUT, DEB, "rdsize: %d\n", rdsize);
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
      XTRACE(INPUT, ALW, "Stopping input thread.\n");
      return;
    }
  }
}

void NMX::processing_thread() {
  nmx_opts = NMXConfig(NMXSettings.ConfigFile);
  init_builder();
  if (!builder_) {
    XTRACE(PROCESS, WAR, "No builder specified, exiting thread\n");
    return;
  }

  Producer eventprod(EFUSettings.KafkaBroker, "NMX_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  Producer monitorprod(EFUSettings.KafkaBroker, "NMX_monitor");
  TrackSerializer trackfb(256, nmx_opts.track_sample_minhits,
                          nmx_opts.time_config.target_resolution_ns());
  HistSerializer histfb;
  NMXHists hists;
  hists.set_cluster_adc_downshift(nmx_opts.cluster_adc_downshift);

  ClusterMatcher matcher(nmx_opts.matcher_max_delta_time);

  TSCTimer global_time, report_timer;

  Event event;
  uint32_t time;
  uint32_t pixelid;

  unsigned int data_index;
  int sample_next_track{0};
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
        auto stats = builder_->process_buffer(eth_ringbuf->getDataBuffer(data_index), len);

        mystats.readouts += stats.valid_hits;
        mystats.readouts_error_bytes += stats.error_bytes; // From srs data parser
        mystats.lost_frames += stats.lost_frames;
        mystats.bad_frames += stats.bad_frames;
        mystats.good_frames += stats.good_frames;

        if (nmx_opts.hit_histograms) {
          hists.bin_hists(builder_->clusterer_x->clusters);
          hists.bin_hists(builder_->clusterer_y->clusters);
        }

        if (!builder_->clusterer_x->empty() && !builder_->clusterer_y->empty()) {
          matcher.merge(0, builder_->clusterer_x->clusters);
          matcher.merge(1, builder_->clusterer_y->clusters);
        }
        matcher.match_end(false);

        while (!matcher.matched_clusters.empty()) {
          //printf("MATCHED_CLUSTERS\n");
          XTRACE(PROCESS, DEB, "event_ready()\n");
          event = matcher.matched_clusters.front();
          matcher.matched_clusters.pop_front();

          //mystats.unclustered = clusterer.unclustered();

          event.analyze(nmx_opts.analyze_weighted,
                        nmx_opts.analyze_max_timebins,
                        nmx_opts.analyze_max_timedif);

          if (nmx_opts.hit_histograms) {
            hists.bin(event);
          }

          if (event.valid()) {
            XTRACE(PROCESS, DEB, "event.good\n");

            mystats.clusters_xy++;

            // TODO: Should it be here or outside of event.valid()?
            if (sample_next_track) {
              sample_next_track = trackfb.add_track(event);
            }

            XTRACE(PROCESS, DEB, "x.center: %d, y.center %d\n",
                   event.x.utpc_center_rounded(), event.y.utpc_center_rounded());

            if (nmx_opts.filter.valid(event)) {

              // printf("\nHave a cluster:\n");
              // event.debug2();

              pixelid = nmx_opts.geometry.pixel2D(event.x.utpc_center_rounded(),
                                                  event.y.utpc_center_rounded());
              if (!nmx_opts.geometry.valid_id(pixelid)) {
                mystats.geom_errors++;
              } else {
                time = static_cast<uint32_t>(event.utpc_time());

                XTRACE(PROCESS, DEB, "time: %d, pixelid %d\n", time, pixelid);

                mystats.tx_bytes += flatbuffer.addevent(time, pixelid);
                mystats.clusters_events++;
              }
            } else { // Does not meet criteria
              /** @todo increments counters when failing this */
              // printf("\nInvalid cluster:\n");
              // event.debug2();
            }
          } else {
            //printf("No valid cluster:\n");
            //event.debug2();
            if (event.x.entries.size() != 0) {
              mystats.clusters_x++;
            } else {
              mystats.clusters_y++;
            }
            mystats.readouts_discarded += event.x.entries.size() + event.y.entries.size();
            mystats.clusters_discarded++;
          }
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      sample_next_track = nmx_opts.send_tracks;

      mystats.tx_bytes += flatbuffer.produce();

      char *txbuffer;
      auto len = trackfb.serialize(&txbuffer);
      if (len != 0) {
        XTRACE(PROCESS, DEB, "Sending tracks with size %d\n", len);
        monitorprod.produce(txbuffer, len);
      }

      if (!hists.isEmpty()) {
        XTRACE(PROCESS, DEB, "Sending histogram for %zu hits and %zu clusters \n",
               hists.hit_count(), hists.cluster_count());
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      if (not runThreads) {

        // TODO flush all clusters?

        XTRACE(INPUT, ALW, "Stopping input thread.\n");
        builder_.reset(); /**< @fixme this is a hack to force ~BuilderSRS() call */
        delete builder_.get(); /**< @fixme see above */
        return;
      }

      report_timer.now();
    }
  }
}

void NMX::init_builder() {
  XTRACE(INIT, ALW, "NMXConfig:\n%s", nmx_opts.debug().c_str());

  auto clusx = std::make_shared<DoroClusterer>(nmx_opts.clusterer_x.max_time_gap,
                                            nmx_opts.clusterer_x.max_strip_gap,
                                            nmx_opts.clusterer_x.min_cluster_size);
  auto clusy = std::make_shared<DoroClusterer>(nmx_opts.clusterer_y.max_time_gap,
                                            nmx_opts.clusterer_y.max_strip_gap,
                                            nmx_opts.clusterer_y.min_cluster_size);

  if (nmx_opts.builder_type == "Hits") {
    XTRACE(INIT, DEB, "Using BuilderHits\n");
    builder_ = std::make_shared<BuilderHits>(nmx_opts.dump_directory,
                                                  nmx_opts.dump_csv, nmx_opts.dump_h5);
    builder_->clusterer_x = clusx;
    builder_->clusterer_y = clusy;
  } else if (nmx_opts.builder_type == "APV") {
    XTRACE(INIT, DEB, "Using BuilderAPV\n");
    builder_ = std::make_shared<BuilderAPV>(nmx_opts.dump_directory,
                                            nmx_opts.dump_csv, nmx_opts.dump_h5);
    builder_->clusterer_x = clusx;
    builder_->clusterer_y = clusy;
  } else if (nmx_opts.builder_type == "VMM2") {
    XTRACE(INIT, DEB, "Using BuilderVMM2\n");
    builder_ = std::make_shared<BuilderVMM2>(
        nmx_opts.time_config, nmx_opts.srs_mappings, clusx, clusy,
        nmx_opts.clusterer_x.hit_adc_threshold, nmx_opts.clusterer_x.max_time_gap,
        nmx_opts.clusterer_y.hit_adc_threshold, nmx_opts.clusterer_y.max_time_gap,
        nmx_opts.dump_directory, nmx_opts.dump_csv, nmx_opts.dump_h5);
  } else if (nmx_opts.builder_type == "VMM3") {
      XTRACE(INIT, DEB, "Using BuilderVMM3\n");
      builder_ = std::make_shared<BuilderVMM3>(
          nmx_opts.time_config, nmx_opts.srs_mappings, clusx, clusy,
          nmx_opts.clusterer_x.hit_adc_threshold, nmx_opts.clusterer_x.max_time_gap,
          nmx_opts.clusterer_y.hit_adc_threshold, nmx_opts.clusterer_y.max_time_gap,
          nmx_opts.dump_directory, nmx_opts.dump_csv, nmx_opts.dump_h5);
  } else {
    XTRACE(INIT, ALW, "Unrecognized builder type in config\n");
  }
}

/** ----------------------------------------------------- */

DetectorFactory<NMX> Factory;
