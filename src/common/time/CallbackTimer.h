//
// Created by Gregory Tucker on 5/28/24.
// Modification of https://stackoverflow.com/a/30425945 by Mikael Persson
//

#ifndef EVENT_FORMATION_UNIT_CALLBACKTIMER_H
#define EVENT_FORMATION_UNIT_CALLBACKTIMER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

class CallbackTimer
{
public:
  CallbackTimer();
  ~CallbackTimer();

  void stop();

  void start(uint64_t interval_msec, std::function<void(void)> func);

  bool is_running() const noexcept;

private:
  std::atomic<bool> _execute;
  std::thread _thd;
};

#endif //EVENT_FORMATION_UNIT_CALLBACKTIMER_H
