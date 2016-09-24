/** Copyright (C) 2016 European Spallation Source */

#include <Counter.h>
#include <EFUArgs.h>
#include <Producer.h>
#include <Socket.h>
#include <Thread.h>
#include <Timer.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <numeric>
#include <queue>
#include <unistd.h> // sleep()

/** Warning GLOBAL VARIABLES */
std::queue<int> queue1;
std::priority_queue<float> queue2;
std::mutex m1, m2, mcout;
/** END WARNING */

/**
 * Input thread - reads from UDP socket and enqueues in FIFO
 */
void input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  Socket::Endpoint local("0.0.0.0", opts->port);

  UDPServer bulkdata(local);
  bulkdata.buflen(opts->buflen);
  if (opts->rcvbuf) {
    bulkdata.setopt(SO_RCVBUF, opts->rcvbuf);
  }
  std::cout << "Socket rcv buffer size: " << bulkdata.getopt(SO_RCVBUF)
            << std::endl;
  std::cout << "Socket sdn buffer size: " << bulkdata.getopt(SO_SNDBUF)
            << std::endl;

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

    /** */
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
void processing_thread(void *args) {
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
void output_thread(void *args) {
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

/**
 * Launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {

  EFUArgs opts(argc, argv);

  Thread t1(12, output_thread, (void *)&opts);
  Thread t2(13, processing_thread, (void *)&opts);
  Thread t3(14, input_thread, (void *)&opts);

#if 1
  Timer stop;
  while (stop.timeus() < opts.stopafter * 1000000) {
    sleep(2);
  }
#endif

  std::cout << "Exiting..." << std::endl;
  exit(1);
  return 0;
}
