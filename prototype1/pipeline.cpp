#include <EFUArgs.h>
#include <Producer.h>
#include <Socket.h>
#include <Thread.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <queue>
#include <unistd.h> // sleep()

typedef std::chrono::high_resolution_clock Clock;

/** Warning GLOBAL VARIABLES */
std::queue<int> queue1;
std::priority_queue<float> queue2;
std::mutex m1, m2, mcout;
/** END WARNING */

/**
 * Input thread - reads from UDP socket and enqueues in FIFO
 */
void input_thread(void *args) {
  uint64_t rx_total = 0;
  uint64_t rx = 0;

  EFUArgs *opts = (EFUArgs *)args;

  Socket::Endpoint local("0.0.0.0", opts->port);
  UDPServer bulkdata(local);
  bulkdata.buflen(opts->buflen);

  auto t1 = Clock::now();
  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    /** this is the processing step */
    if ((rx += bulkdata.receive()) > 0) {
#if 1
      m1.lock();
      queue1.push(5);
      m1.unlock();
#endif
    }
    /** */

    if (usecs >= opts->updint * 1000000) {
      rx_total += rx;
      std::cout << "input     : queue1 size: " << queue1.size()
                << " - rate: " << rx * 8.0 / (usecs / 1000000.0) / 1000000
                << " Mbps" << std::endl;
      rx = 0;
      t1 = Clock::now();
    }
  }
}

/**
 * Processing thread - reads from FIFO and writes to Priority Queue
 */
void processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  std::vector<int> cluster;

  auto t1 = Clock::now();
  int pops = 0;
  int reduct = 0;
  int pops_tot = 0;

  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    /** this is the processing step */
    if (queue1.empty()) {
      usleep(100);
    } else {
      m1.lock();
      queue1.pop(); // At some point take element from queue
      m1.unlock();
      pops++;
      reduct++;
      cluster.push_back(queue1.front());
    }

    if (reduct == opts->reduction) {
      float avg =
          std::accumulate(cluster.begin(), cluster.end(), 0.0) / cluster.size();
      cluster.clear();

      m2.lock();
      queue2.push(avg);
      m2.unlock();
      reduct = 0;
    }
    /** */

    if (usecs >= opts->updint * 1000000) {
      pops_tot += pops;
      mcout.lock();
      std::cout << "processing: queue1 size: " << queue1.size() << " - "
                << pops_tot << " elements - " << pops / (usecs / 1000000.0)
                << " per/s" << std::endl;
      mcout.unlock();
      pops = 0;
      t1 = Clock::now();
    }
  }
}

/**
 * Output thread - reads from Priority Queue and outputs to Kafka cluster
 */
void output_thread(void *args) {

  EFUArgs *opts = (EFUArgs *)args;

  Producer producer(opts->broker, "EFUTestTopic");

  auto t1 = Clock::now();
  int npop = 0;
  int nprod = 0;
  int nprod_tot = 0;
  bool dontproduce = true;
  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

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
    if (!dontproduce) {
      producer.Produce(); /**< use partition 0 now ? */
      nprod++;
    }
    /** */

    if (usecs >= opts->updint * 1000000) {
      mcout.lock();
      std::cout << "output    : queue2 size: " << queue2.size() << " - " << npop
                << " elements - " << nprod / (usecs / 1000000.0)
                << " msgs/s - total msgs: " << nprod_tot << std::endl;
      nprod_tot += nprod;
      nprod = 0;
      mcout.unlock();
      t1 = Clock::now();
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

  while (1) {
    sleep(2);
  }

  return 0;
}
