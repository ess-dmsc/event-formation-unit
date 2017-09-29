/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief CSPEC Detector implementation
 */

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/NewStats.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <multigrid/mgcncs/ChanConv.h>
#include <multigrid/mgcncs/Data.h>
#include <multigrid/mgcncs/MultigridGeometry.h>
//#include <cspec/CSPECEvent.h>
#include <cstring>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <queue>
#include <stdio.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this for time

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector";

class CSPEC : public Detector {
public:
  CSPEC(void *args);
  ~CSPEC();
  void input_thread();
  void processing_thread();
  void output_thread();

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);
  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int event_buffer_max_entries = 200 * eth_buffer_max_entries;
  static const int event_buffer_size = 12;
  static const int kafka_buffer_size = 124000; /**< events */

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  /** Shared between processing_thread and output_thread */
  CircularFifo<unsigned int, event_buffer_max_entries> proc2output_fifo;
  RingBuffer<event_buffer_size> *event_ringbuf;

  NewStats ns{"efu2.cspec."};

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t fifo1_free;
    int64_t pad_a[4]; /**< @todo check alignment*/

    // Processing Counters
    int64_t rx_readouts;
    int64_t rx_error_bytes;
    int64_t rx_discards;
    int64_t rx_idle1;
    int64_t geometry_errors;
    int64_t fifo2_push_errors;
    int64_t fifo2_free;
    int64_t pad_b[1]; /**< @todo check alignment */

    // Output Counters
    int64_t rx_events;
    int64_t rx_idle2;
    int64_t tx_bytes;
  } ALIGN(64) mystats;

  EFUArgs *opts;
};

CSPEC::~CSPEC() {
   delete event_ringbuf;
   delete eth_ringbuf;
}

CSPEC::CSPEC(void *args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.i2pfifo_dropped",           &mystats.fifo1_push_errors);
  ns.create("input.i2pfifo_free",              &mystats.fifo1_free);
  ns.create("processing.rx_readouts",          &mystats.rx_readouts);
  ns.create("processing.rx_error_bytes",       &mystats.rx_error_bytes);
  ns.create("processing.rx_discards",          &mystats.rx_discards);
  ns.create("processing.rx_idle",              &mystats.rx_idle1);
  ns.create("processing.rx_geometry_errors",   &mystats.geometry_errors);
  ns.create("processing.p2ofifo_dropped",      &mystats.fifo2_push_errors);
  ns.create("processing.p2ofifo_free",         &mystats.fifo2_free);
  ns.create("output.rx_events",                &mystats.rx_events);
  ns.create("output.rx_idle",                  &mystats.rx_idle2);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d Ethernet ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);

  XTRACE(INIT, ALW, "Creating %d Event ringbuffers of size %d\n",
         event_buffer_max_entries, event_buffer_size);
  event_ringbuf = new RingBuffer<event_buffer_size>(event_buffer_max_entries);
}

int CSPEC::statsize() { return ns.size(); }

int64_t CSPEC::statvalue(size_t index) { return ns.value(index); }

std::string &CSPEC::statname(size_t index) { return ns.name(index); }

const char *CSPEC::detectorname() { return classname; }

void CSPEC::input_thread() {

  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer cspecdata(local);
  cspecdata.buflen(opts->buflen);
  cspecdata.setbuffers(0, opts->rcvbuf);
  cspecdata.printbuffers();
  cspecdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  Timer stop_timer;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    if ((rdsize = cspecdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                    eth_ringbuf->getmaxbufsize())) > 0) {
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      mystats.fifo1_free = input2proc_fifo.free();
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

void CSPEC::processing_thread() {

  CSPECChanConv conv;
  conv.makewirecal(0, CSPECChanConv::adcsize - 1, 128); // Linear look-up table
  conv.makegridcal(0, CSPECChanConv::adcsize - 1, 96);  // Linear look-up table

  MultiGridGeometry geom(1, 2, 48, 4, 16);

  CSPECData dat(250, &conv, &geom); // Default signal thresholds

  Timer stopafter_clock;
  TSCTimer report_timer;

  unsigned int data_index;
  while (1) {

    // Check for control from mothership (main)
    if (opts->proc_cmd) {
      opts->proc_cmd = 0; /** @todo other means of ipc? */
      XTRACE(PROCESS, INF, "processing_thread loading new calibrations\n");
      conv.load_calibration(opts->wirecal, opts->gridcal);
    }

    mystats.fifo2_free = proc2output_fifo.free();
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      dat.receive(eth_ringbuf->getdatabuffer(data_index),
                  eth_ringbuf->getdatalength(data_index));
      mystats.rx_error_bytes += dat.error;
      mystats.rx_readouts +=
          dat.elems; /**< @todo both valid and invalid events */
      mystats.rx_discards += dat.input_filter();

      for (unsigned int id = 0; id < dat.elems; id++) {
        auto d = dat.data[id];
        if (d.valid) {
          unsigned int event_index = event_ringbuf->getindex();
          if (dat.createevent(d, event_ringbuf->getdatabuffer(event_index)) <
              0) {
            mystats.geometry_errors++;
            assert(mystats.geometry_errors <= mystats.rx_readouts);
          } else {
            if (proc2output_fifo.push(event_index) == false) {
              mystats.fifo2_push_errors++;
              XTRACE(PROCESS, WAR, "Overflow :%" PRIu64 "\n",
                     mystats.fifo2_push_errors);
            } else {
              event_ringbuf->nextbuffer();
            }
          }
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

void CSPEC::output_thread() {
  Producer producer(opts->broker, "C-SPEC_detector");
  FBSerializer flatbuffer(kafka_buffer_size, producer);

  unsigned int event_index;
  Timer stop;
  TSCTimer report_timer;

  while (1) {
    if (proc2output_fifo.pop(event_index) == false) {
      mystats.rx_idle2++;
      usleep(10);
    } else {
      auto evbuf = event_ringbuf->getdatabuffer(event_index);
      XTRACE(OUTPUT, DEB, "output pixel: %d\n", *(uint32_t *)(evbuf + 4));

      mystats.tx_bytes += flatbuffer.addevent(*(uint32_t *)(evbuf + 0),
                                              *(uint32_t *)(evbuf + 4));
      mystats.rx_events++;
    }

    /** Cheking for exit*/
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      mystats.tx_bytes += flatbuffer.produce();

      if (stop.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping output thread, timeus " << stop.timeus()
                  << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *opts) {
    return std::shared_ptr<Detector>(new CSPEC(opts));
  }
};

CSPECFactory Factory;
