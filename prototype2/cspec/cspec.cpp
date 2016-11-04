/** Copyright (C) 2016 European Spallation Source ERIC */

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
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

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector";

class CSPEC : public Detector {
public:
  void input_thread(void *args);
  void processing_thread(void *args);
  void output_thread(void *args);

  static const int buffer_max_entries = 100000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<struct RingBuffer::Data *, buffer_max_entries> fifo;

  /** Shared between processing_thread and output_thread */
  CircularFifo<CSPECEvent *, 300 * buffer_max_entries> fifo2;

  std::mutex eventq_mutex, cout_mutex;

  char kafkabuffer[1000000]; /** @todo not hardcoded */
};

void CSPEC::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer cspecdata(local);
  cspecdata.buflen(opts->buflen);
  cspecdata.setbuffers(0, opts->rcvbuf);
  cspecdata.printbuffers();
  cspecdata.settimeout(0, 100000); // One tenth of a second

  /** Buffer and stats setup */
  RingBuffer ringbuf(buffer_max_entries);

  uint64_t rx = 0;
  uint64_t rx_total = 0;
  uint64_t rxp = 0;
  uint64_t ioverflow = 0;
  int rdsize;

  Timer us_clock, stop_timer;
  TSCTimer report_timer;
  for (;;) {

    /** this is the processing step */
    struct RingBuffer::Data *data = ringbuf.getdatastruct();

    if ((rdsize = cspecdata.receive(data->buffer, ringbuf.getsize())) > 0) {
      rxp++;
      rx += rdsize;
      ringbuf.setdatalength(rdsize);

      if (fifo.push(data) == false) {
        ioverflow++;
      } else {
        ringbuf.nextbuffer();
      }
    }

    /** This is the periodic reporting*/
    if (unlikely(
            (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ))) {
      auto usecs = us_clock.timeus();
      rx_total += rx;

      cout_mutex.lock();
      printf("%" PRIu64 " input     : %8.2f Mb/s, q1: %3d, rxpkt: %12" PRIu64
             ", rxbytes: %12" PRIu64 ", push errors: %" PRIu64 "\n",
             report_timer.timetsc(), rx * 8.0 / usecs, 0, rxp, rx_total,
             ioverflow);
      fflush(stdout);
      cout_mutex.unlock();

      us_clock.now();
      report_timer.now();
      rx = 0;

      if (stop_timer.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping input thread, timeus " << stop_timer.timeus()
                  << std::endl;
        return;
      }
    }
  }
}

void CSPEC::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  uint64_t ierror = 0;
  uint64_t oerror = 0;
  uint64_t idata = 0;
  uint64_t iidle = 0;
  uint64_t idisc = 0;

  CSPECChanConv conv;
  conv.makewirecal(0, CSPECChanConv::adcsize - 1,
                   128); // Linear dummy wire look-up table
  conv.makegridcal(0, CSPECChanConv::adcsize - 1,
                   96); // Linear dummy grid look-up table

  MultiGridGeometry CSPEC(80, 160, 4, 16);

  // CSPECData dat(0, 0, &conv, &CSPEC); // no signal threshold
  CSPECData dat(&conv, &CSPEC); // Default signal thresholds

  Timer us_clock, stopafter_clock;
  TSCTimer report_timer;
  struct RingBuffer::Data *data;
  CSPECEvent testevent(0xffff, 0xffff);

  uint64_t eventcount = 0;
  while (1) {

    if ((fifo.pop(data)) == false) {
      iidle++;
      usleep(10);
    } else {
      dat.receive(data->buffer, data->length);
      ierror += dat.error;
      idata += dat.elems;
      idisc += dat.input_filter();

      for (auto d : dat.data) {
        if (d.valid) {
          auto evt = dat.createevent(d);
          if (fifo2.push(evt) == false) {
            oerror++;
            delete evt;
          }
        }
      }
    }

    /** This is the periodic reporting*/
    if (unlikely(
            (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ))) {
      auto usecs = us_clock.timeus();
      auto rate = idata - eventcount;
      cout_mutex.lock();
      printf("%" PRIu64 " processing: idle: %" PRIu64 ", errors: %" PRIu64
             ", discard: %" PRIu64 ", events: %" PRIu64 ", kevts/s: %" PRIu64
             " , push errors: %" PRIu64 "\n",
             report_timer.timetsc(), iidle, ierror, idisc, idata, rate*1000/usecs,
             oerror);
      fflush(stdout);
      cout_mutex.unlock();

      eventcount = idata;
      us_clock.now();
      report_timer.now();

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
    }
  }
}

void CSPEC::output_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

#ifndef NOKAFKA
  Producer producer(opts->broker, true, "C-SPEC_detector");
#endif
  CSPECEvent *data;
  Timer stop;
  TSCTimer report_timer2;

  uint64_t rxevents = 0;
  uint64_t produce = 0;
  uint64_t idle = 0;
  while (1) {
    if (fifo2.pop(data) == false) {
      idle++;
      usleep(10);
    } else {
      rxevents++;
      produce++;
      delete data;
    }

    /** Add dummy producer of 1M bytes data*/
    if (produce >= 83000) {
#ifndef NOKAFKA
      producer.produce(kafkabuffer, 1000000);
#endif
      produce = 0;
    }

    /** This is the periodic reporting*/

    if (unlikely(
            (report_timer2.timetsc() >= opts->updint * 1000000 * TSC_MHZ))) {
      cout_mutex.lock();
      printf("%" PRIu64 " output    : events: %" PRIu64 ", idle: %" PRIu64
             " \n",
             report_timer2.timetsc(), rxevents, idle);
      fflush(stdout);
      cout_mutex.unlock();

      report_timer2.now();

      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping output thread, timeus " << stop.timeus()
                  << std::endl;
        return;
      }
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
