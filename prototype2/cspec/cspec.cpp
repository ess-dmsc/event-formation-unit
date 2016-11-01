/** Copyright (C) 2016 European Spallation Source ERIC */

#include <common/Detector.h>
#include <common/EFUArgs.h>
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

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector";

class CSPEC : public Detector {
public:
  CSPEC(){};

  ~CSPEC(){};

  void input_thread(void *args);
  void processing_thread(void *args);
  void output_thread(void *args);

  static const int buffer_max_entries = 100000;

private:
  CircularFifo<struct RingBuffer::Data *, buffer_max_entries> fifo;
  CircularFifo<std::shared_ptr<CSPECEvent>, buffer_max_entries> fifo2;
  //std::priority_queue<CSPECEvent> eventq;
  std::mutex eventq_mutex, cout_mutex;
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

      if (unlikely(fifo.push(data) == false)) {
        ioverflow++;
      } else {
        ringbuf.nextbuffer();
      }
    }

    /** This is the periodic reporting*/
    if (unlikely((report_timer.timetsc() / TSC_MHZ >= opts->updint * 1000000))) {
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
  uint64_t idata = 0;
  uint64_t iidle = 0;
  uint64_t idisc = 0;

  CSPECChanConv conv;
  conv.makewirecal(0, CSPECChanConv::adcsize - 1,
                   128); // Linear dummy wire look-up table
  conv.makegridcal(0, CSPECChanConv::adcsize - 1,
                   96); // Linear dummy grid look-up table

  //CSPECData dat(0, 0, &conv); // no signal threshold
  CSPECData dat(&conv); // Default signal thresholds

  Timer stop;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  struct RingBuffer::Data *data;
  CSPECEvent testevent(0xffff, 0xffff);
  while (1) {
    tsc = rdtsc();

    if ((fifo.pop(data)) == false) {
      iidle++;
      usleep(10);
    } else {
      dat.receive(data->buffer, data->length);
      ierror += dat.error;
      idata += dat.elems;
      idisc += dat.input_filter();

      /** Add CSPECEvents to Priority Queue */
      for (auto d : dat.data) {
        if (d.valid) {
          auto evt = dat.createevent(d);
          fifo2.push(evt);
        }
      }
    }

    /** This is the periodic reporting*/
    if (unlikely(((tsc - tsc0) / TSC_MHZ >= opts->updint * 1000000))) {

      cout_mutex.lock();
      printf("%" PRIu64 " processing: idle: %" PRIu64 ", errors: %" PRIu64
             ", discard: %" PRIu64 ", events: %" PRIu64 " \n",
             tsc - tsc0, iidle, ierror, idisc, idata);
      fflush(stdout);
      cout_mutex.unlock();

      tsc0 = rdtsc();
    }

    if (stop.timeus() >= opts->stopafter * 1000000) {
      std::cout << "stopping processing thread, timeus " << stop.timeus()
                << std::endl;
      return;
    }
  }
}


void CSPEC::output_thread(__attribute__((unused)) void *args) {
  //EFUArgs *opts = (EFUArgs *)args;

  std::shared_ptr<CSPECEvent> data;
  while (1) {
    if (fifo2.pop(data) == false) {
      sleep(10);
    } 
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create() { return std::shared_ptr<Detector>(new CSPEC); }
};

CSPECFactory Factory;
