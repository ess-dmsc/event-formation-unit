/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <gdgem/nmx/Geometry.h>
#include <common/HistSerializer.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <gdgem/nmxgen/EventletBuilderH5.h>
#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

#include <gdgem/NMXConfig.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

struct NMXSettingsStruct {
  std::string ConfigFile;
} NMXSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser
      .add_option("-f,--file", NMXSettings.ConfigFile,
                  "NMX (gdgem) specific config file")
      ->group("NMX")
      ->required();
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
  } ALIGN(64) mystats;

  NMXConfig nmx_opts;

  std::shared_ptr<AbstractBuilder> builder_{nullptr};
  void init_builder(std::string jsonfile);
};

PopulateCLIParser PopulateParser{SetCLIArguments};

NMX::~NMX() { printf("NMX detector destructor called\n"); }

NMX::NMX(BaseSettings settings) : Detector("NMX", settings) {
  Stats.setPrefix("efu.nmx");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  Stats.create("rx_packets",           mystats.rx_packets);
  Stats.create("rx_bytes",             mystats.rx_bytes);
  Stats.create("i2pfifo_dropped",      mystats.fifo_push_errors);
  Stats.create("readouts",             mystats.readouts);
  Stats.create("readouts_error_bytes", mystats.readouts_error_bytes);
  Stats.create("readouts_discarded",   mystats.readouts_discarded);
  Stats.create("clusters_discarded",   mystats.clusters_discarded);
  Stats.create("clusters_events",      mystats.clusters_events);
  Stats.create("clusters_x",           mystats.clusters_x);
  Stats.create("clusters_y",           mystats.clusters_y);
  Stats.create("clusters_xy",          mystats.clusters_xy);
  Stats.create("processing_idle",      mystats.processing_idle);
  Stats.create("fifo_seq_errors",      mystats.fifo_seq_errors);
  Stats.create("unclustered",          mystats.unclustered);
  Stats.create("geom_errors",          mystats.geom_errors);

  Stats.create("tx_bytes",             mystats.tx_bytes);
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
  nmxdata.setRecvTimeout(0, 100000 ); /// secs, usecs

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
  init_builder(NMXSettings.ConfigFile);
  if (!builder_) {
    XTRACE(PROCESS, WAR, "No builder specified, exiting thread\n");
    return;
  }

  Geometry geometry;
  geometry.add_dimension(nmx_opts.geometry_x);
  geometry.add_dimension(nmx_opts.geometry_y);

  Producer eventprod(EFUSettings.KafkaBroker, "NMX_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  Producer monitorprod(EFUSettings.KafkaBroker, "NMX_monitor");
  TrackSerializer trackfb(256, nmx_opts.track_sample_minhits);
  HistSerializer histfb;
  NMXHists hists;
  hists.set_cluster_adc_downshift(nmx_opts.cluster_adc_downshift);
  Clusterer clusterer(nmx_opts.cluster_min_timespan);

  TSCTimer global_time, report_timer;

  EventNMX event;
  std::vector<uint16_t> coords{0, 0};
  uint32_t time;
  uint32_t pixelid;

  unsigned int data_index;
  int sample_next_track{0};
  while (1) {
    // mystats.fifo_free = input2proc_fifo.free();
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.processing_idle++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        // printf("received packet with length %d\n", len);
        auto stats = builder_->process_buffer(eth_ringbuf->getDataBuffer(data_index), len, clusterer, hists);

        mystats.readouts += stats.valid_eventlets;
        mystats.readouts_error_bytes += stats.error_bytes; // From srs data parser

        while (clusterer.event_ready()) {
          XTRACE(PROCESS, DEB, "event_ready()\n");
          event = clusterer.get_event();
          mystats.unclustered = clusterer.unclustered();
          hists.bin(event);
          event.analyze(nmx_opts.analyze_weighted,
                        nmx_opts.analyze_max_timebins,
                        nmx_opts.analyze_max_timedif);

          if (event.valid()) {
            XTRACE(PROCESS, DEB, "event.good\n");

            mystats.clusters_xy++;

            if (sample_next_track) {
              sample_next_track = trackfb.add_track(event);
            }

            XTRACE(PROCESS, DEB, "x.center: %d, y.center %d\n",
                   event.x.center_rounded(), event.y.center_rounded());

            if ( (!nmx_opts.enforce_lower_uncertainty_limit ||
                   event.meets_lower_cirterion(nmx_opts.lower_uncertainty_limit)) &&
                 (!nmx_opts.enforce_minimum_eventlets ||
                 (event.x.entries.size() >= nmx_opts.minimum_eventlets &&
                  event.y.entries.size() >= nmx_opts.minimum_eventlets))) {

              // printf("\nHave a cluster:\n");
              // event.debug2();

              coords[0] = event.x.center_rounded();
              coords[1] = event.y.center_rounded();
              pixelid = geometry.to_pixid(coords);
              if (pixelid == 0) {
                mystats.geom_errors++;
              } else {
                time = static_cast<uint32_t>(event.time_start());

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

      sample_next_track = 1;

      mystats.tx_bytes += flatbuffer.produce();

      char *txbuffer;
      auto len = trackfb.serialize(&txbuffer);
      if (len != 0) {
        XTRACE(PROCESS, DEB, "Sending tracks with size %d\n", len);
        monitorprod.produce(txbuffer, len);
      }

      if (!hists.isEmpty()) {
        XTRACE(PROCESS, DEB, "Sending histogram for %zu eventlets and %zu clusters \n",
               hists.eventlet_count(), hists.cluster_count());
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      if (not runThreads) {
        XTRACE(INPUT, ALW, "Stopping input thread.\n");
        builder_.reset(); /**< @fixme this is a hack to force ~BuilderSRS() call */
        delete builder_.get(); /**< @fixme see above */
        return;
      }

      report_timer.now();
    }
  }
}

void NMX::init_builder(std::string jsonfile) {
  nmx_opts = NMXConfig(jsonfile);
  XTRACE(INIT, ALW, "NMXConfig:\n%s", nmx_opts.debug().c_str());

  if (nmx_opts.builder_type == "H5") {
    XTRACE(INIT, DEB, "Make BuilderH5\n");
    builder_ = std::make_shared<BuilderH5>(nmx_opts.dump_directory,
                                           nmx_opts.dump_csv, nmx_opts.dump_h5);
  } else if (nmx_opts.builder_type == "SRS") {
    XTRACE(INIT, DEB, "Make BuilderSRS\n");
    builder_ = std::make_shared<BuilderSRS>(
        nmx_opts.time_config, nmx_opts.srs_mappings, nmx_opts.dump_directory,
        nmx_opts.dump_csv, nmx_opts.dump_h5);
  } else {
    XTRACE(INIT, ALW, "Unrecognized builder type in config\n");
  }
}

/** ----------------------------------------------------- */

class NMXFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new NMX(settings));
  }
};

NMXFactory Factory;
