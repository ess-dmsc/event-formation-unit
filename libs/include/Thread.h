#include <thread>

class Thread {
public:
  Thread(int cpu, void (*func)(void));

private:
  int cpu_;
  std::thread t_;
};
