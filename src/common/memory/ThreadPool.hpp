#pragma once

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
public:
  ThreadPool(size_t threads = std::thread::hardware_concurrency())
      : _threads(threads), _stop(false) {
    for (size_t i = 0; i < _threads; ++i)
      _workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(_queue_mutex);
            _condition.wait(lock, [this] { return _stop || !_tasks.empty(); });
            if (_stop && _tasks.empty())
              return;
            task = std::move(_tasks.front());
            _tasks.pop();
          }

          task();
        }
      });
  }

  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock(_queue_mutex);
      _stop = true;
    }
    _condition.notify_all();
    for (std::thread &worker : _workers)
      worker.join();
  }

  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    {
      std::lock_guard<std::mutex> lock(_queue_mutex);
      if (_stop)
        throw std::runtime_error("ThreadPool stopped");

      _tasks.emplace([task]() { (*task)(); });
    }
    _condition.notify_one();
    return res;
  }

private:
  size_t _threads;
  std::vector<std::thread> _workers;
  std::queue<std::function<void()>> _tasks;
  std::mutex _queue_mutex;
  std::condition_variable _condition;
  std::atomic<bool> _stop;
};

#endif // THREAD_POOL_HPP