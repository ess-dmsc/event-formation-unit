//
// Created by Gregory Tucker on 5/28/24.
// Modification of https://stackoverflow.com/a/30425945 by Mikael Persson
//

#include <common/time/CallbackTimer.h>

CallbackTimer::CallbackTimer()
    :_execute(false)
{}

CallbackTimer::~CallbackTimer() {
    if( _execute.load(std::memory_order_acquire) ) {
        stop();
    };
}

void CallbackTimer::stop()
{
    _execute.store(false, std::memory_order_release);
    if( _thd.joinable() )
        _thd.join();
}

void CallbackTimer::start(uint64_t interval_msec, std::function<void(void)> func)
{
  if( _execute.load(std::memory_order_acquire) ) {
    stop();
  };
  _execute.store(true, std::memory_order_release);
  auto started_at = std::chrono::system_clock::now();
  _thd = std::thread([this, interval_msec, func, started_at](){
    auto next_time = started_at + std::chrono::milliseconds(interval_msec);
    while (_execute.load(std::memory_order_acquire)) {
      func();
      std::this_thread::sleep_until(next_time);
      next_time += std::chrono::milliseconds(interval_msec);
    }
  });
}

bool CallbackTimer::is_running() const noexcept {
  return ( _execute.load(std::memory_order_acquire) && _thd.joinable() );
}