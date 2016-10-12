/** Copyright (C) 2016 European Spallation Source */

#include <CSPECData.h>
#include <Detector.h>
#include <EFUArgs.h>
#include <Socket.h>
#include <Timer.h>
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
  std::mutex i2pqm, mcout;
  std::queue<CSPECData::Data> i2pq;
};

void CSPEC::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  Socket::Endpoint local("0.0.0.0", opts->port);

  UDPServer cspecdata(local);
  cspecdata.buflen(opts->buflen);
  cspecdata.setbuffers(0, opts->rcvbuf);
  cspecdata.printbuffers();

  char buffer[9000];
  uint64_t rx = 0;
  uint64_t rx_total = 0;
  uint64_t rxp = 0;
  int rdsize;
  uint64_t ierrors = 0;

  Timer upd, stop;
  for (;;) {

    /** this is the processing step */
    if ((rdsize = cspecdata.receive(buffer, opts->buflen)) > 0) {
      rxp++;

      CSPECData dat(buffer, rdsize);
      ierrors += dat.ierror;
    }
    rx += rdsize;

    /** This is the periodic reporting*/
    if (rxp % 100 == 0) {
      auto usecs = upd.timeus();
      if (usecs >= opts->updint * 1000000) {
        rx_total += rx;

        mcout.lock();
        printf(
            "input     : %8.2f Mb/s, q1: %3d, rxpkt: %9d, rxbytes: %12" PRIu64
            ", errors: %" PRIu64 "\n",
            rx * 8.0 / usecs, 0, (unsigned int)rxp, rx_total, ierrors);
        fflush(stdout);
        mcout.unlock();

        rx = 0;

        if (stop.timeus() >= opts->stopafter * 1000000) {
          std::cout << "stopping input thread " << std::endl;
          return;
        }

        upd.now();
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
