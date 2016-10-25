/** Copyright (C) 2016 European Spallation Source */

#include <Counter.h>
#include <Detector.h>
#include <EFUArgs.h>
#include <Producer.h>
#include <Socket.h>
#include <Timer.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const char *classname = "NMX Detector";

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

  Socket::Endpoint local("0.0.0.0", opts->port);

  UDPServer bulkdata(local);
  bulkdata.buflen(opts->buflen);
  bulkdata.setbuffers(0, opts->rcvbuf);
  bulkdata.printbuffers();

  char buffer[9000];
  unsigned int seqno = 1;
  unsigned int seqno_exp = 1;
  unsigned int lost = 0;

  uint64_t rx_total = 0;
  uint64_t rx = 0;

  Timer upd, stop;
  for (;;) {

    /** this is the processing step */
    if ((rx += bulkdata.receive(buffer, opts->buflen)) > 0) {
      std::memcpy(&seqno, buffer, sizeof(seqno));

      lost += (seqno - seqno_exp);
      seqno_exp = seqno + 1;

      m1.lock();
      queue1.push(seqno);
      m1.unlock();
    } else {
      std::cout << "Receive 0 " << std::endl;
    }

    /** This is the periodic reporting*/
    if (seqno % 100 == 0) {
      auto usecs = upd.timeus();
      if (usecs >= opts->updint * 1000000) {
        rx_total += rx;

        mcout.lock();
        printf(
            "input     : %8.2f Mb/s, q1: %3d, rxpkt: %9d, rxbytes: %12" PRIu64
            ", PER: %6.3e\n",
            rx * 8.0 / (usecs / 1000000.0) / 1000000, (int)queue1.size(), seqno,
            rx_total, 1.0 * lost / seqno);
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

/**
 * Processing thread - reads from FIFO and writes to Priority Queue
 */
void NMX::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  Counter<int> cluster;

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
  Producer producer(opts->broker, kafka, "EFUTestTopic");

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
        producer.Produce();
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
  NMX *create() {
    cout << "    making NMX" << endl;
    return new NMX;
  }
};

NMXFactory Factory;
