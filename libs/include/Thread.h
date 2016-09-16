#include <thread>

/** Wrapper class for pthread creation. Also 'locks' pthread to
 * specified lcore.
 *
 */
class Thread {

public:
  Thread(int cpu, void (*func)(void));
  Thread(int cpu, void (*func)(void *a), void *arg);

private:
  void SetAffinity(int lcore);

  int lcore_;
  std::thread t_;
  void *args_;
};
