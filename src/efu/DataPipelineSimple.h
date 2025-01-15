#pragma once

#include "efu/ThreadPool.hpp"
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <vector>

namespace data_pipeline_simple {
template <typename Input, typename Output>
using StageFunction = std::function<std::future<Output>(Input)>;

class Pipeline {
private:
  std::vector<std::function<std::future<void *>(void *)>> stages;

public:
  template <typename Input, typename Output>
  void addStage(StageFunction<Input, Output> func) {
    stages.push_back([func](void *in) -> std::future<void *> {
      return std::async(std::launch::async, [func, in]() {
        std::unique_ptr<Input> typedInput(static_cast<Input *>(in));
        std::future<Output> result = func(*typedInput);
        void *out = new Output(result.get());
        return out;
      });
    });
  }

  template <typename StartType> std::future<void *> run(StartType initialData) {
    return ThreadPool::getInstance().enqueue([this, initialData]() {
          void *data = new StartType(initialData);

          for (auto &stage : stages) {
            std::future<void *> result = stage(data);
            data = result.get();
          }

          return data;
        });
  }

  template <typename FinalType> FinalType getResult(void *finalData) {
    FinalType result = *static_cast<FinalType *>(finalData);
    delete static_cast<FinalType *>(finalData);
    return result;
  }
};

} // namespace data_pipeline_simple