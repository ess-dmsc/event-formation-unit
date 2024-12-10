// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Lock-free queue implementation for producer-consumer pattern
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>
#include <cstddef>

class LockFreeQueueBase {
public:
  virtual ~LockFreeQueueBase() = default;
  // Define any common interface methods if necessary
};

template <typename T> class LockFreeQueue : public LockFreeQueueBase {
public:
  LockFreeQueue(size_t size)
      : size(size), buffer(new T[size]), head(0), tail(0) {}

  ~LockFreeQueue() { delete[] buffer; }

  bool enqueue(const T &&data) {
    size_t current_tail = tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % size;

    if (next_tail == head.load(std::memory_order_acquire)) {
      return false; // Queue is full
    }

    buffer[current_tail] = data;
    tail.store(next_tail, std::memory_order_release);
    return true;
  }

  bool enqueue(const T &data) { return enqueue(std::move(data)); }

  bool dequeue(T &result) {
    size_t current_head = head.load(std::memory_order_relaxed);

    if (current_head == tail.load(std::memory_order_acquire)) {
      return false; // Queue is empty
    }

    result = buffer[current_head];
    head.store((current_head + 1) % size, std::memory_order_release);
    return true;
  }

private:
  size_t size;
  T *buffer;
  std::atomic<size_t> head;
  std::atomic<size_t> tail;
};