#include <Socket.h>
#include <Thread.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <unistd.h> // sleep()

typedef std::chrono::high_resolution_clock Clock;

std::queue<int> queue1;
std::priority_queue<int> queue2;
std::mutex m1, m2, mcout;

/**
 * Input thread - reads from UDP socket and enqueues in FIFO
 */
void input_thread(void) {
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  Socket::Endpoint local("0.0.0.0", 9000);
  UDPServer bulkdata(local);
  auto t1 = Clock::now();
  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    /** this is the processing step */
    if ((rx += bulkdata.Receive()) > 0) {
#if 1
      m1.lock();
      queue1.push(5);
      m1.unlock();
#endif
    }
    /** */

    if (usecs >= 1000000) {
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
void processing_thread(void) {
  auto t1 = Clock::now();
  int reduction = 80;
  int npops = 0;
  int tpops = 0;

  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    /** this is the processing step */
    if (queue1.empty()) {
      usleep(100);
    } else {
      m1.lock();
      queue1.pop();
      m1.unlock();
      npops++;
    }

    if (npops == 80) {
      npops = 0;
      tpops += 80;
      m2.lock();
      queue2.push(6);
      m2.unlock();
    }
    /** */

    if (usecs >= 1000000) {
      mcout.lock();
      std::cout << "processing: queue1 size: " << queue1.size() << " - "
                << tpops << " elements" << std::endl;
      mcout.unlock();
      t1 = Clock::now();
    }
  }
}

/**
 * Output thread - reads from Priority Queue and outputs to Kafka cluster
 */
void output_thread(void) {
  auto t1 = Clock::now();
  int npop = 0;
  for (;;) {
    auto t2 = Clock::now();
    auto usecs =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    /** this is the processing step */
    if (queue2.empty()) {
      usleep(100);
    } else {
      m2.lock();
      queue2.pop();
      m2.unlock();
      npop++;
    }
    /** */

    if (usecs >= 1000000) {
      mcout.lock();
      std::cout << "output    : queue2 size: " << queue2.size() << " - " << npop
                << " elements" << std::endl;
      mcout.unlock();
      t1 = Clock::now();
    }
  }
}

/**
 * Launch pipeline threads, then sleep forever
 */
int main(int argc, char *argv[]) {
  Thread t1(12, output_thread);
  Thread t2(13, processing_thread);
  Thread t3(14, input_thread);

  while (1) {
    sleep(2);
  }

  return 0;
}
