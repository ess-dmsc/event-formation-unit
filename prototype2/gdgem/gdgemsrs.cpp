/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/NewStats.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <gdgem/nmx/Clusterer.h>
#include <gdgem/vmm2srs/EventletBuilder.h>
#include <gdgem/vmm2srs/HistSerializer.h>
#include <gdgem/vmm2srs/NMXVMM2SRSData.h>
#include <gdgem/vmm2srs/TrackSerializer.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector for VMM2 data via SRS";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class NMXVMM2SRS : public Detector {
public:
  NMXVMM2SRS(void *args);
  void input_thread();
  void processing_thread();

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 124000; /**< events */

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  NewStats ns{
      "efu2.nmxvmm2srs."}; // Careful also uding this for other NMX pipeline

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5];

    int64_t rx_idle1;
    int64_t rx_readouts;
    int64_t rx_errbytes;
    int64_t rx_discards;
    int64_t rx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;

  EFUArgs *opts;
};

NMXVMM2SRS::NMXVMM2SRS(void *args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.dropped",                   &mystats.fifo1_push_errors);
  ns.create("processing.idle",                 &mystats.rx_idle1);
  ns.create("processing.rx_readouts",          &mystats.rx_readouts);
  ns.create("processing.rx_discards",          &mystats.rx_discards);
  ns.create("processing.rx_errbytes",          &mystats.rx_errbytes);
  ns.create("output.rx_events",                &mystats.rx_events);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

int NMXVMM2SRS::statsize() { return ns.size(); }

int64_t NMXVMM2SRS::statvalue(size_t index) { return ns.value(index); }

std::string &NMXVMM2SRS::statname(size_t index) { return ns.name(index); }

void NMXVMM2SRS::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer nmxdata(local);
  nmxdata.buflen(opts->buflen);
  nmxdata.setbuffers(0, opts->rcvbuf);
  nmxdata.printbuffers();
  nmxdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  Timer stop_timer;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                  eth_ringbuf->getmaxbufsize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (stop_timer.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping input thread, timeus " << stop_timer.timeus()
                  << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

void NMXVMM2SRS::processing_thread() {

  Producer eventprod(opts->broker, "NMX_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);
  Producer monitorprod(opts->broker, "NMX_monitor");
  HistSerializer histfb(1500);
  TrackSerializer trackfb(256);

  NMXVMM2SRSData data(1125);

  Time time_interpreter;
  time_interpreter.set_tac_slope(125); /**< @todo get from slow control? */
  time_interpreter.set_bc_clock(40);   /**< @todo get from slow control? */
  time_interpreter.set_trigger_resolution(
      3.125); /**< @todo get from slow control? */
  time_interpreter.set_target_resolution(0.5); /**< @todo not hardcode */

  Geometry geometry_intepreter; /**< @todo not hardocde chip mappings */
  geometry_intepreter.define_plane(0, {{1, 0}, {1, 1}});
  geometry_intepreter.define_plane(1, {{1, 14}, {1, 15}});

  EventletBuilder builder(time_interpreter, geometry_intepreter);

  Clusterer clusterer(30); /**< @todo not hardocde */

  Timer stopafter_clock;
  TSCTimer global_time, report_timer;
  EventNMX event;
  unsigned int data_index;
  int sample_next_track = 0;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      data.receive(eth_ringbuf->getdatabuffer(data_index),
                   eth_ringbuf->getdatalength(data_index));
      if (data.elems > 0) {
        builder.process_readout(data, clusterer);

        mystats.rx_readouts += data.elems;
        mystats.rx_errbytes += data.error;

        while (clusterer.event_ready()) {
          XTRACE(PROCESS, DEB, "event_ready()\n");
          event = clusterer.get_event();
          event.analyze(true, 3, 7); /**< @todo not hardocde */
          if (event.good()) {
            XTRACE(PROCESS, DEB, "event.good\n");

            if (sample_next_track) {
              sample_next_track = trackfb.add_track(event, 6);
            }
            mystats.rx_events++;

            XTRACE(PROCESS, DEB, "x.center: %f, y.center %f\n", event.x.center,
                   event.y.center);
            int pixelid = (int)event.x.center + (int)event.y.center * 256;

            assert(pixelid < 65535);

            // printf("event time: %" PRIu64 "\n", event.time_start());
            mystats.tx_bytes +=
                flatbuffer.addevent((uint32_t)event.time_start(), pixelid);
            mystats.rx_events++;
          } else {
            mystats.rx_discards +=
                event.x.entries.size() + event.y.entries.size();
          }
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {
      // printf("timetsc: %" PRIu64 "\n", global_time.timetsc());

      sample_next_track = 1;

      flatbuffer.produce();

      char *txbuffer;
      auto len = trackfb.serialize(&txbuffer);
      if (len != 0) {
        XTRACE(PROCESS, ALW, "Sending tracks with size %d\n", len);
        monitorprod.produce(txbuffer, len);
      }

      if (data.xyhist_elems != 0) {
        XTRACE(PROCESS, ALW, "Sending histogram with %d readouts\n",
               data.xyhist_elems);
        char *txbuffer;
        auto len = histfb.serialize(&data.xyhist[0][0], &data.xyhist[1][0],
                                    1500, &txbuffer);
        monitorprod.produce(txbuffer, len);
        data.hist_clear();
      }

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

/** ----------------------------------------------------- */

class NMXVMM2SRSFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new NMXVMM2SRS(args));
  }
};

NMXVMM2SRSFactory Factory;
