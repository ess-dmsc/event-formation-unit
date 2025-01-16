#pragma once
#include <condition_variable>
#include <efu/Graylog.h>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stopWorkers;

  ThreadPool(size_t threads); // Constructor should be private for singleton

public:
  static ThreadPool &
  getInstance(); // Public accessor for the singleton instance

  void stop() {
    LOG(MAIN, Sev::Info, "Stopping ThreadPool workers");
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stopWorkers = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
  }

  ~ThreadPool() {
    stop();
  }
};

// Implementation of the constructor, destructor, and getInstance method
inline ThreadPool::ThreadPool(size_t threads) : stopWorkers(false) {
  for (size_t i = 0; i < threads; ++i)
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex);
          condition.wait(lock,
                         [this] { return stopWorkers || !tasks.empty(); });
          if (stopWorkers && tasks.empty())
            return;
          task = std::move(tasks.front());
          tasks.pop();
        }
        task();
      }
    });
}

inline ThreadPool &ThreadPool::getInstance() {
  static ThreadPool instance(std::thread::hardware_concurrency());
  return instance;
}