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

const char *classname = "CSPEC Detector";

/** ----------------------------------------------------- */

class CSPEC : public Detector {
public:
  CSPEC(){};

  ~CSPEC(){};

  void input_thread(void *a);

private:
  std::mutex mcout;
};

void CSPEC::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  Socket::Endpoint local("0.0.0.0", opts->port);

  UDPServer cspecdata(local);
  cspecdata.buflen(opts->buflen);
  cspecdata.setbuffers(0, opts->rcvbuf);
  cspecdata.printbuffers();
  cspecdata.settimeout(1); // One second

  RingBuffer ringbuf(9000, 100);
  uint64_t rx = 0;
  uint64_t rx_total = 0;
  uint64_t rxp = 0;
  uint64_t ierror = 0;
  uint64_t idata = 0;
  int rdsize;

  CSPECData dat;
  Timer stop;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  for (;;) {
    tsc = rdtsc();
    /** this is the processing step */
    char * bufptr = ringbuf.getbuffer();
    if ((rdsize = cspecdata.receive(bufptr, opts->buflen)) > 0) {
      rxp++;
      dat.receive(bufptr, rdsize);
      ierror += dat.error;
      idata += dat.elems;
      dat.input_filter();
      rx += rdsize;
    }

    /** This is the periodic reporting*/
    if (unlikely(((tsc - tsc0) / 2400 >= opts->updint * 1000000))) {
      rx_total += rx;

      mcout.lock();
      printf("%" PRIu64 " input     : %8.2f Mb/s, q1: %3d, rxpkt: %12" PRIu64
             ", rxbytes: %12" PRIu64 ", errors: %" PRIu64 ", events: %" PRIu64
             "\n",
             tsc - tsc0, rx * 8.0 / ((tsc - tsc0) / 2400), 0, rxp, rx_total,
             ierror, idata);
      fflush(stdout);
      mcout.unlock();

      tsc0 = rdtsc();
      rx = 0;

      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping input thread, timeus" << stop.timeus()
                  << std::endl;
        return;
      }
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  Detector *create() { return new CSPEC; }
};

CSPECFactory Factory;
