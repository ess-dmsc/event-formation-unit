/** Copyright (C) 2016 European Spallation Source */

#include <Counter.h>
#include <EFUArgs.h>
#include <Producer.h>
#include <Socket.h>
#include <Thread.h>
#include <Timer.h>
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

  uint64_t rx_total = 0;
  uint64_t rx = 0;

  Timer upd, stop;
  for (;;) {

    /** this is the processing step */
    if ((rx += bulkdata.receive()) > 0) {
      m1.lock();
      queue1.push(5);
      m1.unlock();
    }
    /** */
    auto usecs = upd.timeus();
    if (usecs >= opts->updint * 1000000) {
      rx_total += rx;
      std::cout << "input     : queue1 size: " << queue1.size()
                << " - rate: " << rx * 8.0 / (usecs / 1000000.0) / 1000000
                << " Mbps" << std::endl;
      rx = 0;

      if (stop.timeus() >= opts->stopafter * 1000000) {
        std::cout << "stopping input thread " << std::endl;
        return;
      }

      upd.now();
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
    auto usecs = upd.timeus();
    if (usecs >= opts->updint * 1000000) {
      pops_tot += pops;
      mcout.lock();
      std::cout << "processing: queue1 size: " << queue1.size() << " - "
                << pops_tot << " elements - " << pops / (usecs / 1000000.0)
                << " per/s" << std::endl;
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
      mcout.lock();
      std::cout << "output    : queue2 size: " << queue2.size() << " - " << npop
                << " elements - " << nprod / (usecs / 1000000.0)
                << " msgs/s - total msgs: " << nprod_tot << std::endl;
      mcout.unlock();
      nprod_tot += nprod;
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

  Timer stop;
  while (stop.timeus() < opts.stopafter * 1000000) {
    sleep(2);
  }

  std::cout << "Exiting..." << std::endl;

  return 0;
}
