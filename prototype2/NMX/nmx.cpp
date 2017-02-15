/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/TSCTimer.h>
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
  NMX();
  void input_thread(void *args);
  void processing_thread(void *args);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  char kafkabuffer[kafka_buffer_size];
};


NMX::NMX() {
  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
      eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

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
      opts->stat.stats.rx_packets++;
      opts->stat.stats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      opts->stat.stats.fifo1_free = input2proc_fifo.free();
      if (input2proc_fifo.push(eth_index) == false) {
        opts->stat.stats.fifo1_push_errors++;

        XTRACE(INPUT, WAR, "Overflow :%" PRIu64 "\n",
               opts->stat.stats.fifo1_push_errors);
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

  //NMXData dat;

  Timer stopafter_clock;
  TSCTimer report_timer;

  unsigned int data_index;
  while (1) {
    opts->stat.stats.fifo1_free = input2proc_fifo.free();
    if ((input2proc_fifo.pop(data_index)) == false) {
      opts->stat.stats.rx_idle1++;
      usleep(10);
    } else {
      //dat.receive(eth_ringbuf->getdatabuffer(data_index),
      //            eth_ringbuf->getdatalength(data_index));
      opts->stat.stats.rx_readouts += 1;
      opts->stat.stats.rx_error_bytes += 0;
      opts->stat.stats.rx_discards += 1;

      kafkabuffer[0] = 0; /**< @todo for now , to please clang */
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
  std::shared_ptr<Detector> create() {
    return std::shared_ptr<Detector>(new NMX);
  }
};

NMXFactory Factory;
