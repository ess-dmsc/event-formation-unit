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
#include <variant>
#include <vector>

namespace data_pipeline {

class Pipeline; // Forward declaration

class PipelineStageBase {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~PipelineStageBase() = default;

  virtual std::shared_ptr<LockFreeQueueBase> getInputQueue() const = 0;
  virtual std::shared_ptr<LockFreeQueueBase> getOutputQueue() const = 0;
  virtual void
  connectOutput(std::shared_ptr<LockFreeQueueBase> nextOutputQueue) = 0;

protected:
  std::future<void> future;

  PipelineStageBase() = default;

private:
  friend class Pipeline;
};

template <typename In, typename Out>
class PipelineStage : public PipelineStageBase {

public:
  PipelineStage(std::function<Out(In)> func,
                size_t inputQueueSize = defaultQueueSize,
                size_t outputQueueSize = defaultQueueSize)
      : func(func),
        inputQueue(std::make_shared<LockFreeQueue<In>>(inputQueueSize)),
        outputQueue(std::make_shared<LockFreeQueue<Out>>(outputQueueSize)),
        stopFlag(false) {}

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

  // Implement connectOutput
  void
  connectOutput(std::shared_ptr<LockFreeQueueBase> nextOutputQueue) override {
    // Cast to the appropriate type
    auto castedQueue =
        std::static_pointer_cast<LockFreeQueue<Out>>(nextOutputQueue);
    outputQueue = castedQueue;
  }

  std::shared_ptr<LockFreeQueueBase> getInputQueue() const override {
    return inputQueue;
  }

  std::shared_ptr<LockFreeQueueBase> getOutputQueue() const override {
    return outputQueue;
  }

private:
  static const size_t defaultQueueSize = 1024;

  std::function<Out(In)> func;
  std::shared_ptr<LockFreeQueue<In>> inputQueue;
  std::shared_ptr<LockFreeQueue<Out>> outputQueue;
  std::atomic_bool stopFlag;

  friend class Pipeline;
};

class Pipeline {
public:
  ~Pipeline() { stop(); }

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

  template <typename In> std::shared_ptr<LockFreeQueue<In>> getInputQueue() {
    auto inputQueue = stages.front()->getInputQueue();
    auto castedQueue = std::static_pointer_cast<LockFreeQueue<In>>(inputQueue);
    return castedQueue;
  }

  template <typename Out>
  std::shared_ptr<LockFreeQueue<Out>> getOutputQueue() {
    auto outputQueue = stages.back()->getOutputQueue();
    auto castedQueue =
        std::static_pointer_cast<LockFreeQueue<Out>>(outputQueue);
    return castedQueue;
  }

private:
  // Private constructor to enforce the use of the builder
  Pipeline(std::vector<std::unique_ptr<PipelineStageBase>> stages)
      : stages(std::move(stages)) {}

  std::vector<std::unique_ptr<PipelineStageBase>> stages;

  // Grant access to PipelineBuilder
  template <typename... StageTypes> friend class PipelineBuilder;
};

template <typename... StageTypes> class PipelineBuilder {
public:
  PipelineBuilder() = default;

  // Add and connect a new stage
  template <typename NewStageType, typename... Args>
  auto addStage(Args &&...args) {
    static_assert(std::is_base_of_v<PipelineStageBase, NewStageType>,
                  "Stage must derive from PipelineStageBase");

    auto stage = std::make_unique<NewStageType>(std::forward<Args>(args)...);

    if (!stages_.empty()) {
      // Get the output queue of the last stage
      auto &previousStage = stages_.back();
      auto previousOutputQueue = previousStage->getOutputQueue();

      // Get the input queue of the new stage
      auto newInputQueue = stage->getInputQueue();

      // Connect the previous stage's output to the new stage's input
      previousStage->connectOutput(newInputQueue);
    }

    stages_.push_back(std::move(stage));

    // Return a new builder with the new stage type added
    return PipelineBuilder<StageTypes..., NewStageType>(std::move(stages_));
  }

  // Build the pipeline
  Pipeline build() { return Pipeline(std::move(stages_)); }

private:
  std::vector<std::unique_ptr<PipelineStageBase>> stages_;

  // Private constructor used when adding a stage
  PipelineBuilder(std::vector<std::unique_ptr<PipelineStageBase>> stages)
      : stages_(std::move(stages)) {}

  // Grant access to other instances of PipelineBuilder
  template <typename... OtherStageTypes> friend class PipelineBuilder;
};

} // namespace data_pipeline