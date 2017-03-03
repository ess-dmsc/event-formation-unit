/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/ParserClusterer.h>
#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
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
#include <stdio.h>
#include <unistd.h>

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class NMX : public Detector {
public:
  NMX(void *args);
  void input_thread(void *args);
  void processing_thread(void *args);

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  char kafkabuffer[kafka_buffer_size];

  NewStats ns{"efu2.nmx."};

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
    int64_t rx_events;
    int64_t tx_bytes;

  } ALIGN(64) mystats;

  EFUArgs *opts;
};

NMX::NMX(void *UNUSED args) {
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
  ns.create("output.rx_events",                &mystats.rx_events);
  ns.create("output.tx_bytes",                 &mystats.tx_bytes);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

int NMX::statsize() { return ns.size(); }

int64_t NMX::statvalue(size_t index) { return ns.value(index); }

std::string &NMX::statname(size_t index) { return ns.name(index); }

void NMX::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

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

void NMX::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  assert(opts != NULL);

#ifndef NOKAFKA
  Producer producer(opts->broker, true, "NMX_detector");
#endif

  ParserClusterer parser;

  Timer stopafter_clock;
  TSCTimer report_timer;

  unsigned int data_index;
  int evtoff = 0;
  while (1) {
    mystats.fifo1_free = input2proc_fifo.free();
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      parser.parse(eth_ringbuf->getdatabuffer(data_index),
                   eth_ringbuf->getdatalength(data_index));

      unsigned int readouts = eth_ringbuf->getdatalength(data_index) /
                              (sizeof(uint32_t) * 4); /**< @todo not hardocde */

      mystats.rx_readouts += readouts;
      mystats.rx_error_bytes += 0;

      while (parser.event_ready()) {
        auto event = parser.get();
        event.analyze(true, 3, 7);
        if (event.good) {
          // image[c2d(static_cast<uint32_t>(event.x.center),
          //          static_cast<uint32_t>(event.y.center))]++;
          mystats.rx_events++;

          int time = 42; /**< @todo get time from ? */
          int pixelid = (int)event.x.center + (int)event.y.center * 256;

          std::memcpy(kafkabuffer + evtoff, &time, sizeof(time));
          std::memcpy(kafkabuffer + evtoff + 4, &pixelid, sizeof(pixelid));
          evtoff += 8;

          if (evtoff >= kafka_buffer_size / 10 - 20) {
            assert(evtoff < kafka_buffer_size);

#ifndef NOKAFKA
            producer.produce(kafkabuffer, evtoff);
            mystats.tx_bytes += evtoff;
#endif
            evtoff = 0;
          }
        } else {
          mystats.rx_discards +=
              event.x.entries.size() + event.y.entries.size();
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

/** ----------------------------------------------------- */

class NMXFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new NMX(args));
  }
};

NMXFactory Factory;
