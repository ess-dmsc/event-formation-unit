/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <EFUArgs.h>
#include <RingBuffer.h>
#include <SPSCFifo.h>
#include <Socket.h>
#include <Timer.h>
#include <cspec/CSPECData.h>
#include <gccintel.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <unistd.h>

using namespace std;
using namespace memory_sequential_consistent;

const char *classname = "CSPEC Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable
// const int TSC_MHZ = 2400; // dmsc-parallel (?)

/** ----------------------------------------------------- */

class CSPEC : public Detector {
public:
  CSPEC(){};

  ~CSPEC(){};

  void input_thread(void *args);
  void processing_thread(void *args);

  static const int buffer_max_entries = 1000;

private:
  CircularFifo<struct RingBuffer::Data *, buffer_max_entries> fifo;
  std::mutex mcout;
};

void CSPEC::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  /** Connection setup */
  Socket::Endpoint local("0.0.0.0", opts->port);
  UDPServer cspecdata(local);
  cspecdata.buflen(opts->buflen);
  cspecdata.setbuffers(0, opts->rcvbuf);
  cspecdata.printbuffers();
  cspecdata.settimeout(0, 100000); // One tenth of a second

  /** Buffer and stats setup */
  RingBuffer ringbuf(9000, buffer_max_entries);

  uint64_t rx = 0;
  uint64_t rx_total = 0;
  uint64_t rxp = 0;
  uint64_t ioverflow = 0;
  int rdsize;

  Timer stop;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  for (;;) {
    tsc = rdtsc();

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
    if (unlikely(((tsc - tsc0) / TSC_MHZ >= opts->updint * 1000000))) {
      rx_total += rx;

      mcout.lock();
      printf("%" PRIu64 " input     : %8.2f Mb/s, q1: %3d, rxpkt: %12" PRIu64
             ", rxbytes: %12" PRIu64 ", push errors: %" PRIu64 "\n",
             tsc - tsc0, rx * 8.0 / ((tsc - tsc0) / TSC_MHZ), 0, rxp, rx_total,
             ioverflow);
      fflush(stdout);
      mcout.unlock();

      tsc0 = rdtsc();
      rx = 0;

      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping input thread, timeus " << stop.timeus()
                  << std::endl;
        return;
      }
    }
  }
}

void CSPEC::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  CSPECData dat;
  uint64_t ierror = 0;
  uint64_t idata = 0;
  uint64_t iidle = 0;

  Timer stop;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  struct RingBuffer::Data *data;
  while (1) {
    tsc = rdtsc();

    if ((fifo.pop(data)) == false) {
      iidle++;
      usleep(10);
    } else {
      dat.receive(data->buffer, data->length);
      ierror += dat.error;
      idata += dat.elems;
      dat.input_filter();
    }

    /** This is the periodic reporting*/
    if (unlikely(((tsc - tsc0) / TSC_MHZ >= opts->updint * 1000000))) {

      mcout.lock();
      printf("%" PRIu64 " processing: idle: %" PRIu64 ", errors: %" PRIu64
             ",events: %" PRIu64 " \n",
             tsc - tsc0, iidle, ierror, idata);
      fflush(stdout);
      mcout.unlock();

      tsc0 = rdtsc();
    }

    if (stop.timeus() >= opts->stopafter * 1000000) {
      std::cout << "stopping processing thread, timeus " << stop.timeus()
                << std::endl;
      return;
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  Detector *create() { return new CSPEC; }
};

CSPECFactory Factory;
