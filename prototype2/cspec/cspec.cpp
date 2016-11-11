/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cstring>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CSPECData.h>
#include <cspec/CSPECEvent.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <unistd.h>

/** @file
 *
 *  @brief CSPEC Detector implementation
 */

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB


using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector";

class CSPEC : public Detector {
public:
  CSPEC();
  void input_thread(void *args);
  void processing_thread(void *args);
  void output_thread(void *args);

  static const int eth_buffer_max_entries = 10000;
  static const int eth_buffer_size = 9000;
  static const int event_buffer_max_entries = 20 * eth_buffer_max_entries;
  static const int event_buffer_size = 12;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> * eth_ringbuf;

  /** Shared between processing_thread and output_thread */
  CircularFifo<unsigned int, event_buffer_max_entries> proc2output_fifo;
  RingBuffer<event_buffer_size> * event_ringbuf;

  std::mutex eventq_mutex, cout_mutex;

  char kafkabuffer[kafka_buffer_size];
};

CSPEC::CSPEC() {
  XTRACE(INIT, INF, "Creating CSPEC ringbuffers %d\n", 5); /** @todo make this work */
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  event_ringbuf = new RingBuffer<event_buffer_size>(event_buffer_max_entries);
}

void CSPEC::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;


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
    if ((rdsize = cspecdata.receive(eth_ringbuf->getdatabuffer(eth_index), eth_ringbuf->getmaxbufsize())) > 0) {
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);
      opts->stat.i.rx_packets++;
      opts->stat.i.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      opts->stat.i.fifo_free = input2proc_fifo.free();
      if (input2proc_fifo.push(eth_index) == false) {
        opts->stat.i.push_errors++;

        XTRACE(INPUT, WAR, "Overflow :%lu\n", opts->stat.i.push_errors);
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


void CSPEC::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  CSPECChanConv conv;
  conv.makewirecal(0, CSPECChanConv::adcsize - 1, 128); // Linear look-up table
  conv.makegridcal(0, CSPECChanConv::adcsize - 1,  96); // Linear look-up table

  MultiGridGeometry CSPEC(80, 160, 4, 16);

  // CSPECData dat(0, 0, &conv, &CSPEC); // Custom signal thresholds
  CSPECData dat(&conv, &CSPEC); // Default signal thresholds

  Timer stopafter_clock;
  TSCTimer report_timer;

  unsigned int data_index;
  while (1) {

    if ((input2proc_fifo.pop(data_index)) == false) {
      opts->stat.p.idle++;
      usleep(10);
    } else {
      dat.receive(eth_ringbuf->getdatabuffer(data_index), eth_ringbuf->getdatalength(data_index));
      opts->stat.p.rx_errors += dat.error;
      opts->stat.p.rx_events += dat.elems;
      opts->stat.p.rx_discards += dat.input_filter();

      opts->stat.p.fifo_free = proc2output_fifo.free();
      for (auto d : dat.data) {
        if (d.valid) {
          unsigned int event_index = event_ringbuf->getindex();
          dat.createevent(d, event_ringbuf->getdatabuffer(event_index));
          if (proc2output_fifo.push(event_index) == false) {
            opts->stat.p.push_errors++;;
          } else {
            event_ringbuf->nextbuffer();
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

void CSPEC::output_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

#ifndef NOKAFKA
  Producer producer(opts->broker, true, "C-SPEC_detector");
#endif

  unsigned int event_index;
  Timer stop;
  TSCTimer report_timer;

  uint64_t produce = 0;
  while (1) {
    if (proc2output_fifo.pop(event_index) == false) {
      opts->stat.o.idle++;
      usleep(10);
    } else {
      std::memcpy(kafkabuffer + produce, event_ringbuf->getdatabuffer(event_index), 8 /**< @todo not hardcode */ );
      opts->stat.o.rx_events++;
      produce += 12; /**< @todo should match actual data size */
    }

    /** Produce when enough data has been accumulated */
    if (produce >= kafka_buffer_size - 1000) {
      assert(produce < kafka_buffer_size);
#ifndef NOKAFKA
      producer.produce(kafkabuffer, kafka_buffer_size);
      opts->stat.o.tx_bytes += kafka_buffer_size;
#endif
      produce = 0;
    }

    /** Cheking for exit*/
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

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
  std::shared_ptr<Detector> create() {
    return std::shared_ptr<Detector>(new CSPEC);
  }
};

CSPECFactory Factory;
