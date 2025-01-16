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
#include <chrono>
#include <common/debug/Log.h>
#include <common/memory/LockFreeQueue.h>
#include <cstdint>
#include <future>
#include <iostream>
#include <vector>

namespace data_pipeline {

class Pipeline; // Forward declaration

/// \brief Base class for pipeline stages.
///
/// This class provides the interface for pipeline stages.
/// It should be inherited by specific pipeline stage implementations.
class PipelineStageBase {
public:
  /// \brief Start the pipeline stages.
  virtual void start() = 0;

  /// \brief Stop the pipeline stages.
  virtual void stop() = 0;

  virtual ~PipelineStageBase() = default;

  /// \brief Get the input queue.
  /// \return Shared pointer to the input \ref LockFreeQueueBase.
  virtual std::shared_ptr<LockFreeQueueBase> getInputQueue() const = 0;

  /// \brief Get the output queue.
  /// \return Shared pointer to the output \ref LockFreeQueueBase.
  virtual std::shared_ptr<LockFreeQueueBase> getOutputQueue() const = 0;

  /// \brief Connect the output queue to the next stage's input.
  /// \param nextOutputQueue Shared pointer to the next stage's input queue.
  virtual void
  connectNextStage(std::shared_ptr<LockFreeQueueBase> NextStageInputQueue) = 0;

  virtual uint64_t getPerformanceCounter() const = 0;

protected:
  std::future<void> Future;

  PipelineStageBase() = default;

private:
  friend class Pipeline;
};

/// \brief Template class for a pipeline stage.
///
/// \tparam In Type of input data.
/// \tparam Out Type of output data.
template <typename In, typename Out>
class PipelineStage : public PipelineStageBase {

public:
  /// \brief Constructor for the pipeline stage.
  /// \param func Processing function that takes \ref In and returns \ref Out.
  /// \param inputQueueSize Size of the input queue (default 1024).
  /// \param outputQueueSize Size of the output queue (default 1024).
  PipelineStage(std::function<Out(In)> Func,
                size_t InputQueueSize = DEFAULT_QUEUE_SIZE,
                size_t OutputQueueSize = DEFAULT_QUEUE_SIZE)
      : Func(Func),
        InputQueue(std::make_shared<LockFreeQueue<In>>(InputQueueSize)),
        OutputQueue(std::make_shared<LockFreeQueue<Out>>(OutputQueueSize)),
        StopFlag(false) {}

  /// \brief Start the pipeline stage.
  void start() override {
    Future = std::async(std::launch::async, [this]() {
      In Input;
      while (!StopFlag.load(std::memory_order_acquire)) {
        auto current_time = std::chrono::steady_clock::now();
        if (InputQueue->dequeue(Input)) {

          Out Output = Func(std::move(Input));

          auto end_time = std::chrono::steady_clock::now();
          PerformanceCounter.fetch_add(
              std::chrono::duration_cast<std::chrono::microseconds>(
                  end_time - current_time)
                  .count(),
              std::memory_order_release);

          while (!OutputQueue->enqueue(std::move(Output))) {
            std::this_thread::yield();
          }
        } else {
          std::this_thread::yield();
        }
      }
    });
  }

  /// \brief Stop the pipeline stage.
  void stop() override {
    StopFlag.store(true, std::memory_order_release);
    if (Future.valid()) {
      Future.get();
    }
  }

  /// \brief Connect the output queue to the next stage's input.
  /// \param nextOutputQueue Shared pointer to the next stage's input queue.
  void connectNextStage(
      std::shared_ptr<LockFreeQueueBase> NextStageInputQueue) override {
    // Cast to the appropriate type
    auto CastedQueue =
        std::static_pointer_cast<LockFreeQueue<Out>>(NextStageInputQueue);
    OutputQueue = CastedQueue;
  }

  /// \brief Get the input queue.
  /// \return Shared pointer to the input \ref LockFreeQueueBase.
  std::shared_ptr<LockFreeQueueBase> getInputQueue() const override {
    return InputQueue;
  }

  /// \brief Get the output queue.
  /// \return Shared pointer to the output \ref LockFreeQueueBase.
  std::shared_ptr<LockFreeQueueBase> getOutputQueue() const override {
    return OutputQueue;
  }

  // Add getter functions for wait times
  uint64_t getPerformanceCounter() const override {
    return PerformanceCounter.load(std::memory_order_acquire);
  }

private:
  static const size_t DEFAULT_QUEUE_SIZE = 1024;

  std::function<Out(In)> Func;
  std::atomic<int64_t> PerformanceCounter;
  std::shared_ptr<LockFreeQueue<In>> InputQueue;
  std::shared_ptr<LockFreeQueue<Out>> OutputQueue;
  std::atomic_bool StopFlag;

  friend class Pipeline;
};

/// \brief Manages the pipeline composed of multiple stages.
class Pipeline {
public:
  /// \brief Destructor that stops the pipeline first.
  ~Pipeline() { stop(); }

  /// \brief Start all stages of the pipeline.
  void start() {
    for (auto &Stage : Stages) {
      Stage->start();
    }
  }

  /// \brief Stop all stages of the pipeline.
  void stop() {
    for (auto &Stage : Stages) {
      Stage->stop();
    }
  }

  /// \brief Check if the pipeline is healthy.
  /// \return True if all stages are running correctly.
  bool isPipelineHealty() {
    bool AllStageHealty = true;
    for (auto &Stage : Stages) {
      if (!Stage->Future.valid() || Stage->Future.wait_for(std::chrono::seconds(
                                        0)) == std::future_status::ready) {
        AllStageHealty = false;
      }
    }
    return AllStageHealty;
  }

  uint64_t getStagePerformance(const int &StageNumber) const {
    if (StageNumber < 0 || StageNumber >= static_cast<int>(Stages.size())) {
      throw std::out_of_range("StageNumber is out of range");
    }
    return Stages[StageNumber]->getPerformanceCounter();
  }

  /// \brief Get errors that occurred in the pipeline.
  /// \return Vector of runtime errors from pipeline stages.
  std::vector<std::runtime_error> getPipelineErros() {
    std::vector<std::runtime_error> ErrorList;

    for (auto &Stage : Stages) {
      if (Stage->Future.valid() && Stage->Future.wait_for(std::chrono::seconds(
                                       0)) == std::future_status::ready) {
        try {
          Stage->Future.get();
        } catch (const std::exception &E) {
          ErrorList.emplace_back(E.what());
        }
      }
    }
    return ErrorList;
  }

  /// \brief Get the number of running stages.
  /// \return Number of stages that are still running.
  size_t getRunningStages() {
    size_t RunningStages = 0;
    for (auto &Stage : Stages) {
      if (Stage->Future.valid() && Stage->Future.wait_for(std::chrono::seconds(
                                       0)) != std::future_status::ready) {
        RunningStages++;
      }
    }
    return RunningStages;
  }

  /// \brief Get the input queue of the first stage.
  /// \tparam In Type of input data.
  /// \return Shared pointer to the input \ref LockFreeQueue.
  template <typename In> std::shared_ptr<LockFreeQueue<In>> getInputQueue() {
    auto InputQueue = Stages.front()->getInputQueue();
    auto CastedQueue = std::static_pointer_cast<LockFreeQueue<In>>(InputQueue);
    return CastedQueue;
  }

  /// \brief Get the output queue of the last stage.
  /// \tparam Out Type of output data.
  /// \return Shared pointer to the output \ref LockFreeQueue.
  template <typename Out> std::shared_ptr<LockFreeQueue<Out>> getOutputQueue() {
    auto OutputQueue = Stages.back()->getOutputQueue();
    auto CastedQueue =
        std::static_pointer_cast<LockFreeQueue<Out>>(OutputQueue);
    return CastedQueue;
  }

private:
  /// \brief Private constructor to enforce the use of the builder.
  /// \param stages Vector of unique pointers to \ref PipelineStageBase.
  Pipeline(std::vector<std::unique_ptr<PipelineStageBase>> Stages)
      : Stages(std::move(Stages)) {}

  std::vector<std::unique_ptr<PipelineStageBase>> Stages;

  /// \brief Grant access to \ref PipelineBuilder.
  template <typename... StageTypes> friend class PipelineBuilder;
};

/// \brief Builder class for creating a \ref Pipeline.
///
/// \tparam StageTypes Variadic template parameters for stage types.
template <typename... StageTypes> class PipelineBuilder {
public:
  PipelineBuilder() = default;

  /// \brief Add and connect a new stage to the pipeline.
  /// \tparam NewStageType Type of the new stage.
  /// \tparam Args Parameter pack for the new stage constructor arguments.
  /// \param args Arguments to forward to the new stage constructor.
  /// \return A new \ref PipelineBuilder with the new stage type added.
  template <typename NewStageType, typename... Args>
  auto addStage(Args &&...args) {
    static_assert(std::is_base_of_v<PipelineStageBase, NewStageType>,
                  "Stage must derive from PipelineStageBase");

    auto Stage = std::make_unique<NewStageType>(std::forward<Args>(args)...);

    if (!Stages_.empty()) {
      // Get the output queue of the last stage
      auto &PreviousStage = Stages_.back();

      // Get the input queue of the new stage
      auto NewInputQueue = Stage->getInputQueue();

      // Connect the previous stage's output to the new stage's input
      PreviousStage->connectNextStage(NewInputQueue);
    }

    Stages_.push_back(std::move(Stage));

    // Return a new builder with the new stage type added
    return PipelineBuilder<StageTypes..., NewStageType>(std::move(Stages_));
  }

  /// \brief Build the pipeline with the added stages.
  /// \return A constructed \ref Pipeline.
  Pipeline build() {
    if (Stages_.empty()) {
      throw std::runtime_error("No stages added to pipeline");
    }

    return Pipeline(std::move(Stages_));
  }

private:
  std::vector<std::unique_ptr<PipelineStageBase>> Stages_;

  /// \brief Private constructor used when adding a stage.
  /// \param stages Vector of unique pointers to \ref PipelineStageBase.
  PipelineBuilder(std::vector<std::unique_ptr<PipelineStageBase>> Stages)
      : Stages_(std::move(Stages)) {}

  /// \brief Grant access to other instances of \ref PipelineBuilder.
  template <typename... OtherStageTypes> friend class PipelineBuilder;
};

} // namespace data_pipeline