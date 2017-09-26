
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
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <sonde/ideas/Data.h>
#include <stdio.h>
#include <unistd.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "SoNDe detector using IDEA readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class SONDEIDEA : public Detector {
public:
  SONDEIDEA(void *args);
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
      "efu2.sonde."}; //

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5];

    // Processing and Output counters
    int64_t rx_idle1;
    int64_t rx_events;
    int64_t rx_geometry_errors;
    int64_t tx_bytes;
  } ALIGN(64) mystats;

  EFUArgs *opts;
};

SONDEIDEA::SONDEIDEA(void *args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.dropped",                   &mystats.fifo1_push_errors);
  ns.create("processing.idle",                 &mystats.rx_idle1);
  ns.create("processing.rx_events",            &mystats.rx_events);
  ns.create("processing.rx_geometry_errors",   &mystats.rx_geometry_errors);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d SONDE Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

int SONDEIDEA::statsize() { return ns.size(); }

int64_t SONDEIDEA::statvalue(size_t index) { return ns.value(index); }

std::string &SONDEIDEA::statname(size_t index) { return ns.name(index); }

void SONDEIDEA::input_thread() {
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

void SONDEIDEA::processing_thread() {
  SoNDeGeometry geometry;

  IDEASData ideasdata(&geometry);
  Producer eventprod(opts->broker, "SKADI_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  Timer stopafter_clock;
  TSCTimer global_time, report_timer;

  unsigned int data_index;

  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      int events = ideasdata.receive(eth_ringbuf->getdatabuffer(data_index),
        eth_ringbuf->getdatalength(data_index));

      mystats.rx_geometry_errors += ideasdata.errors;
      mystats.rx_events += ideasdata.events;

      for (int i = 0; i < events; i++) {
          mystats.tx_bytes += flatbuffer.addevent(ideasdata.data[i].time, ideasdata.data[i].pixel_id);
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {
      // printf("timetsc: %" PRIu64 "\n", global_time.timetsc());

      flatbuffer.produce();

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

/** ----------------------------------------------------- */

class SONDEIDEAFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new SONDEIDEA(args));
  }
};

SONDEIDEAFactory Factory;
