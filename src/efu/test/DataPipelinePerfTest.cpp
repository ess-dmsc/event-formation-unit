// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Tests for the data pipeline implementation
///
//===----------------------------------------------------------------------===//

#include <chrono>
#include <common/testutils/TestBase.h>
#include <efu/DataPipeline.h>
#include <efu/DataPipelineSimple.h>
#include <memory>
#include <numeric>
#include <random>

#include <vector>

std::vector<double> generateRandomVector(long size) {
  std::vector<double> randomNumbers;
  randomNumbers.reserve(size);
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(1, 50);
  for (int i = 0; i < size; ++i) {
    randomNumbers.push_back(dist(rng));
  }
  return randomNumbers;
}

std::vector<double> longAndExpMath(std::vector<double> data) {
  std::vector<double> factorData(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    // Perform intense math calculation, e.g., exponential and logarithm
    factorData[i] = std::exp(data[i]);
  }

  for (size_t i = 0; i < data.size(); ++i) {
    factorData[i] = std::log(factorData[i]);
  }

  // Reduce the array by deleting every second value
  std::vector<double> reducedData;
  for (size_t i = 0; i < data.size(); i += 2) {
    reducedData.push_back(factorData[i]);
  }

  return reducedData;
}

long long calculateSum(const std::vector<double> &data) {
  long long sum = 0;
  for (const auto &value : data) {
    sum += value;
  }
  return sum;
}

class DataPipelinePerfTest : public TestBase {
protected:
  void SetUp() override {}
};

TEST_F(DataPipelinePerfTest, PipelineWithLargeData) {
  const int numElements = 300;
  const int timeoutMs = 30000;
  const int checkIntervalMs = 10;

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<long, std::vector<double>>>(
              [](long length) -> std::vector<double> {
                return generateRandomVector(length);
              })
          .addStage<data_pipeline::PipelineStage<std::vector<double>,
                                                 std::vector<double>>>(
              longAndExpMath)
          .addStage<data_pipeline::PipelineStage<std::vector<double>,
                                                 std::vector<double>>>(
              longAndExpMath)
          .addStage<
              data_pipeline::PipelineStage<std::vector<double>, long long>>(
              calculateSum)
          .build();

  auto inputQueue = pipeline.getInputQueue<long>();
  auto outputQueue = pipeline.getOutputQueue<long long>();

  pipeline.start();

  auto future = std::async(std::launch::async, [&]() {
    std::vector<long long> results;
    long long result;
    while (results.size() < numElements) {
      if (outputQueue->dequeue(result)) {
        results.push_back(result);
      } else {
        std::this_thread::yield(); // Yield to avoid busy waiting if the queue
                                   // is empty
      }
    }

    return results;
  });

  for (int i = 0; i < numElements; ++i) {
    bool enqueued = false;
    while (!enqueued) {
      enqueued = inputQueue->enqueue(1000000); // Example input size
      std::this_thread::yield(); // Yield to avoid busy waiting if the queue is
                                 // full
    }
  }

  int elapsedMs = 0;

  // Wait for the future to be ready or the timeout to be reached
  while (future.wait_for(std::chrono::milliseconds(checkIntervalMs)) !=
             std::future_status::ready &&
         elapsedMs < timeoutMs) {
    elapsedMs += checkIntervalMs;
  }

  std::cout << "Elapsed time: " << elapsedMs << " ms" << std::endl;

  std::vector<long long> results;
  if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    results = future.get();
  }

  pipeline.stop();

  ASSERT_EQ(results.size(), numElements);
}

TEST_F(DataPipelinePerfTest, PipelineWithLargeDataSimple)
{
  const int numElements = 300;

  auto pipeline = data_pipeline_simple::Pipeline();

  pipeline.addStage<int,
                    std::vector<double>>(
      [](int input) -> std::future<std::vector<double>>
      {
        return std::async(std::launch::async, [input]()
                          { return generateRandomVector(input); });
      });

  pipeline.addStage<std::vector<double>, std::vector<double>>(
      [](std::vector<double> input) -> std::future<std::vector<double>>
      {
        return std::async(std::launch::async, [input = std::move(input)]()
                          { return longAndExpMath(input); });
      });

  pipeline.addStage<std::vector<double>, std::vector<double>>(
      [](std::vector<double> input) -> std::future<std::vector<double>>
      {
        return std::async(std::launch::async, [input = std::move(input)]()
                          { return longAndExpMath(input); });
      });

  pipeline.addStage<std::vector<double>, long long>(
      [](std::vector<double> input) -> std::future<long long>
      {
        return std::async(std::launch::async, [input = std::move(input)]()
                          { return calculateSum(input); });
      });

  int startTimer = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  std::vector<std::future<void *>> futures;
  for (int i = 0; i < numElements; ++i)
  {
    futures.push_back(pipeline.run(1000000));
  }

  std::vector<long long> results;
  for (auto &future : futures)
  {
    void *finalData = future.get();
    results.push_back(pipeline.getResult<long long>(finalData));
  }

  int endTimer = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();

  std::cout << "Elapsed time: " << endTimer - startTimer << " ms" << std::endl;

  ASSERT_EQ(results.size(), numElements);
}

TEST_F(DataPipelinePerfTest, PipelineWithLargeDataSingleStage) {
  const int numElements = 300;
  const int timeoutMs = 30000;
  const int checkIntervalMs = 10;

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<long, long long>>(
              [](long length) -> long long {
                // Combined operations using predefined functions
                std::vector<double> randomVec = generateRandomVector(length);
                std::vector<double> processedVec =
                    longAndExpMath(std::move(randomVec));
                long long sum =
                    calculateSum(longAndExpMath(std::move(processedVec)));
                return sum;
              })
          .build();

  auto inputQueue = pipeline.getInputQueue<long>();
  auto outputQueue = pipeline.getOutputQueue<long>();

  pipeline.start();

  auto future = std::async(std::launch::async, [&]() {
    std::vector<long> results;
    long result;
    while (results.size() < numElements) {
      if (outputQueue->dequeue(result)) {
        results.push_back(result);
      } else {
        std::this_thread::yield(); // Yield to avoid busy waiting if the queue
                                   // is empty
      }
    }
    return results;
  });

  for (int i = 0; i < numElements; ++i) {
    bool enqueued = false;
    while (!enqueued) {
      enqueued = inputQueue->enqueue(1000000); // Example input size
      std::this_thread::yield(); // Yield to avoid busy waiting if the queue is
                                 // full
    }
  }

  int elapsedMs = 0;

  // Wait for the future to be ready or the timeout to be reached
  while (future.wait_for(std::chrono::milliseconds(checkIntervalMs)) !=
             std::future_status::ready &&
         elapsedMs < timeoutMs) {
    elapsedMs += checkIntervalMs;
  }

  std::cout << "Elapsed time: " << elapsedMs << " ms" << std::endl;

  std::vector<long> results;
  if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    results = future.get();
  }

  pipeline.stop();

  std::cout << "Single stage executed " << numElements << " times\n";

  ASSERT_EQ(results.size(), numElements);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}