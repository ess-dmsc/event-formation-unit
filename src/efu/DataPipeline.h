// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of a data pipeline using lock-free queues
///
//===----------------------------------------------------------------------===//


#include <atomic>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include <common/memory/LockFreeQueue.h>

// Pipeline stage
template <typename In, typename Out> class PipelineStage {
public:
  PipelineStage(std::function<Out(In)> func, LockFreeQueue<In> *inputQueue,
                LockFreeQueue<Out> *outputQueue)
      : func(func), inputQueue(inputQueue), outputQueue(outputQueue),
        stopFlag(false) {}

  void start() {
    thread = std::thread([this]() {
      In input;
      while (!stopFlag.load(std::memory_order_acquire)) {
        if (inputQueue->dequeue(input)) {
          Out output = func(input);
          while (!outputQueue->enqueue(std::move(output))) {
            std::this_thread::yield();
          }
        } else {
          std::this_thread::yield();
        }
      }
    });
  }

  void stop() {
    stopFlag.store(true, std::memory_order_release);
    if (thread.joinable()) {
      thread.join();
    }
  }

private:
  std::function<Out(In)> func;
  LockFreeQueue<In> *inputQueue;
  LockFreeQueue<Out> *outputQueue;
  std::atomic<bool> stopFlag;
  std::thread thread;
};

template <typename T>
class Pipeline {
public:
    Pipeline(size_t queueSize) : queueSize(queueSize) {}

    void addTransformation(std::function<T(T)> transformation) {
        transformations.push_back(transformation);
    }

    void build() {
        if (transformations.empty()) {
            throw std::runtime_error("No transformations added to the pipeline.");
        }

        // Create queues
        for (size_t i = 0; i <= transformations.size(); ++i) {
            queues.push_back(std::make_unique<LockFreeQueue<T>>(queueSize));
        }

        // Create pipeline stages
        for (size_t i = 0; i < transformations.size(); ++i) {
            stages.push_back(std::make_unique<PipelineStage<T, T>>(
                transformations[i], 
                queues[i].get(), 
                queues[i + 1].get()
            ));
        }
    }

    void start() {
        for (auto& stage : stages) {
            stage->start();
        }
    }

    void stop() {
        for (auto& stage : stages) {
            stage->stop();
        }
    }

    LockFreeQueue<T>* getInputQueue() {
        return queues.front().get();
    }

    LockFreeQueue<T>* getOutputQueue() {
        return queues.back().get();
    }

private:
    size_t queueSize;
    std::vector<std::function<T(T)>> transformations;
    std::vector<std::unique_ptr<LockFreeQueue<T>>> queues;
    std::vector<std::unique_ptr<PipelineStage<T, T>>> stages;
};