/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Producer.h>
#include <cstring>
#include <iostream>
#include <libs/include/StatCounter.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <libs/include/gccintel.h>
#include <memory>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "NMX Detector";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class NMX : public Detector {
public:
  void input_thread(void *args);

  void processing_thread(void *args);

  void output_thread(void *args);

  NMX() { cout << "    NMX created" << endl; };

  ~NMX() { cout << "    NMX destroyed" << endl; };

private:
  std::queue<int> queue1;
  std::priority_queue<float> queue2;
  std::mutex m1, m2, mcout;
};

void NMX::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);

  UDPServer bulkdata(local);
  bulkdata.buflen(opts->buflen);
  bulkdata.setbuffers(0, opts->rcvbuf);
  bulkdata.printbuffers();
  bulkdata.settimeout(0, 100000); // One tenth of a second

  char buffer[9000];
  unsigned int seqno = 1;
  unsigned int seqno_exp = 1;
  unsigned int lost = 0;

  uint64_t rx_total = 0;
  uint64_t rx = 0;
  int rdsize;

  Timer upd, stop;
  uint64_t tsc0 = rdtsc();
  uint64_t tsc;
  for (;;) {
    tsc = rdtsc();

    /** this is the processing step */
    if ((rdsize = bulkdata.receive(buffer, opts->buflen)) > 0) {
      std::memcpy(&seqno, buffer, sizeof(seqno));
      rx += rdsize;
      lost += (seqno - seqno_exp);
      seqno_exp = seqno + 1;

      m1.lock();
      queue1.push(seqno);
      m1.unlock();
    }

    /** This is the periodic reporting*/
    if (unlikely(((tsc - tsc0) / TSC_MHZ >= opts->updint * 1000000))) {
      auto usecs = upd.timeus();
      rx_total += rx;

      mcout.lock();
      printf("input     : %8.2f Mb/s, q1: %3d, rxpkt: %9d, rxbytes: %12" PRIu64
             ", PER: %6.3e\n",
             rx * 8.0 / usecs, (int)queue1.size(), seqno, rx_total,
             1.0 * lost / seqno);
      fflush(stdout);
      mcout.unlock();

      upd.now();
      tsc0 = rdtsc();
      rx = 0;
      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping input thread " << std::endl;
        return;
      }
    }
  }
}

/**
 * Processing thread - reads from FIFO and writes to Priority Queue
 */
void NMX::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  StatCounter<int> cluster;

  int pops = 0;
  int reduct = 0;
  int pops_tot = 0;

  Timer upd, stop;
  for (;;) {

    /** this is the processing step */
    if (queue1.empty()) {
      usleep(100);
    } else {

      m1.lock();
      queue1.pop(); // At some point take element from queue
      m1.unlock();

      pops++;
      reduct++;
      cluster.add(queue1.front());
    }

    if (reduct == opts->reduction) {

      float avg = cluster.avg();
      cluster.clear();

      m2.lock();
      queue2.push(avg);
      m2.unlock();
      reduct = 0;
    }
    /** */
    if ((pops % 100) == 0) {
      auto usecs = upd.timeus();
      if (usecs >= opts->updint * 1000000) {
        pops_tot += pops;

        mcout.lock();
        printf("processing: q1: %3d, elements: %9d, rate: %7" PRIu64
               " elems/s\n",
               (int)queue1.size(), pops_tot, pops / (usecs / 1000000));
        fflush(stdout);
        mcout.unlock();

        pops = 0;

        if (stop.timeus() >= opts->stopafter * 1000000) {
          std::cout << "stopping processing thread " << std::endl;
          return;
        }

        upd.now();
      }
    }
  }
}

/**
 * Output thread - reads from Priority Queue and outputs to Kafka cluster
 */
void NMX::output_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  bool kafka = opts->kafka;
#ifndef NOKAFKA
  Producer producer(opts->broker, kafka, "EFUTestTopic");
#endif

  int npop = 0;
  int nprod = 0;
  int nprod_tot = 0;
  bool dontproduce = true;

  Timer upd, stop;
  for (;;) {

    /** this is the processing step */
    if ((dontproduce = queue2.empty())) {
      usleep(100);
    } else {
      m2.lock();
      queue2.pop();
      m2.unlock();
      npop++;
    }

    /** Produce message */
    if (kafka) {
      if (!dontproduce) {
#ifndef NOKAFKA
        producer.produce();
#endif
        nprod++;
      }
    }
    /** */
    auto usecs = upd.timeus();
    if (usecs >= opts->updint * 1000000) {
      nprod_tot += nprod;

      mcout.lock();
      printf("output    : q2: %3d, elements: %9d, rate: %7" PRIu64 " elems/s\n",
             (int)queue2.size(), npop, nprod / (usecs / 1000000));
      fflush(stdout);
      mcout.unlock();

      nprod = 0;

      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping output thread " << std::endl;
        return;
      }

      upd.now();
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
