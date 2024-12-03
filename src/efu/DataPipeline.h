// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of a data pipeline using lock-free queues
///
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>
#include <common/debug/Log.h>
#include <common/memory/LockFreeQueue.h>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace data_pipeline {

class Pipeline; // Forward declaration

class PipelineStageBase {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~PipelineStageBase() = default;

protected:
  std::future<void> future;

  PipelineStageBase() = default;

private:
  friend class Pipeline;
};

template <typename In, typename Out>
class PipelineStage : public PipelineStageBase {

public:
  void start() override {
    future = std::async(std::launch::async, [this]() {
      In input;
      while (!stopFlag.load(std::memory_order_acquire)) {
        if (inputQueue->dequeue(input)) {
          Out output;
          output = func(input);
          while (!outputQueue->enqueue(std::move(output))) {
            std::this_thread::yield();
          }
        } else {
          std::this_thread::yield();
        }
      }
    });
  }

  void stop() override {
    stopFlag.store(true, std::memory_order_release);
    if (future.valid()) {
      future.get();
    }
  }

  void connectOutput(std::shared_ptr<LockFreeQueue<Out>> nextOutputQueue) {
    outputQueue = nextOutputQueue;
  }

  std::shared_ptr<LockFreeQueue<In>> getInputQueue() const {
    return inputQueue;
  }

  std::shared_ptr<LockFreeQueue<Out>> getOutputQueue() const {
    return outputQueue;
  }

private:
  static const size_t defaultQueueSize = 1024;

  PipelineStage(std::function<Out(In)> func,
                size_t inputQueueSize = defaultQueueSize,
                size_t outputQueueSize = defaultQueueSize)
      : func(func),
        inputQueue(std::make_shared<LockFreeQueue<In>>(inputQueueSize)),
        outputQueue(std::make_shared<LockFreeQueue<Out>>(outputQueueSize)),
        stopFlag(false) {}

  std::function<Out(In)> func;
  std::shared_ptr<LockFreeQueue<In>> inputQueue;
  std::shared_ptr<LockFreeQueue<Out>> outputQueue;
  std::atomic_bool stopFlag;

  friend class Pipeline;
};

class Pipeline {
public:
  ~Pipeline() { stop(); }

  template <typename In, typename Out>
  PipelineStage<In, Out> &createNewStage(std::function<Out(In)> func) {
    auto stage = std::unique_ptr<PipelineStage<In, Out>>(
        new PipelineStage<In, Out>(func));
    auto *stagePtr = stage.get();
    stages.push_back(std::move(stage));
    return *stagePtr;
  }

  void start() {
    for (auto &stage : stages) {
      stage->start();
    }
  }

  void stop() {
    for (auto &stage : stages) {
      stage->stop();
    }
  }

  bool isPipelineHealty() {
    bool allStageHealty = true;
    for (auto &stage : stages) {
      if (!stage->future.valid() || stage->future.wait_for(std::chrono::seconds(
                                        0)) == std::future_status::ready) {
        allStageHealty = false;
      }
    }
    return allStageHealty;
  }

  std::vector<std::runtime_error> getPipelineErros() {
    std::vector<std::runtime_error> errorList;

    for (auto &stage : stages) {
      if (stage->future.valid() && stage->future.wait_for(std::chrono::seconds(
                                       0)) == std::future_status::ready) {
        try {
          stage->future.get();
        } catch (const std::exception &e) {
          errorList.emplace_back(e.what());
        }
      }
    }
    return errorList;
  }

  size_t getRunningStages() {
    size_t runningStages = 0;
    for (auto &stage : stages) {
      if (stage->future.valid() && stage->future.wait_for(std::chrono::seconds(
                                       0)) != std::future_status::ready) {
        runningStages++;
      }
    }
    return runningStages;
  }

private:
  std::vector<std::unique_ptr<PipelineStageBase>> stages;
};

} // namespace data_pipeline