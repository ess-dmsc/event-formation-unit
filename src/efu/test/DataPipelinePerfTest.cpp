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
#include <memory>
#include <numeric>
#include <random>
#include <vector>

std::vector<double> generateRandomVector(int size) {
  std::vector<double> randomNumbers;
  randomNumbers.reserve(size);
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int> dist(1, 50);
  for (int i = 0; i < size; ++i) {
    randomNumbers.push_back(dist(rng));
  }
  return randomNumbers;
}

std::unique_ptr<std::vector<double>>
longAndExpMath(std::unique_ptr<std::vector<double>> data) {
  double *factorData = new double[data->size()];
  for (size_t i = 0; i < data->size(); ++i) {
    // Perform intense math calculation, e.g., exponential and logarithm
    factorData[i] = std::exp(data->at(i));
  }

  for (size_t i = 0; i < data->size(); ++i) {
    factorData[i] = std::log(factorData[i]);
  }

  // Reduce the array by deleting every second value
  std::vector<double> reducedData;
  for (size_t i = 0; i < data->size(); i += 2) {
    reducedData.push_back(factorData[i]);
  }

  delete[] factorData;
  factorData = new double[reducedData.size()];
  std::copy(reducedData.begin(), reducedData.end(), factorData);
  std::unique_ptr<std::vector<double>> result =
      std::make_unique<std::vector<double>>(factorData,
                                            factorData + reducedData.size());
  delete[] factorData;
  return result;
}

long long calculateSum(const std::unique_ptr<std::vector<double>> data) {
  double *arrayData = new double[data->size()];
  std::copy(data->begin(), data->end(), arrayData);

  long long sum = 0;
  for (size_t i = 0; i < data->size(); ++i) {
    sum += arrayData[i];
  }

  delete[] arrayData;
  return sum;
}

class DataPipelinePerfTest : public TestBase {
protected:
  void SetUp() override {}
};

TEST_F(DataPipelinePerfTest, PipelineWithLargeData) {
  const int numElements = 20;
  const int timeoutMs = 30000;
  const int checkIntervalMs = 10;

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<
              long, std::unique_ptr<std::vector<double>>>>(
              [](long length) -> std::unique_ptr<std::vector<double>> {
                return std::make_unique<std::vector<double>>(
                    generateRandomVector(static_cast<int>(length)));
              })
          .addStage<data_pipeline::PipelineStage<
              std::unique_ptr<std::vector<double>>,
              std::unique_ptr<std::vector<double>>>>(longAndExpMath)
          .addStage<data_pipeline::PipelineStage<
              std::unique_ptr<std::vector<double>>,
              std::unique_ptr<std::vector<double>>>>(longAndExpMath)
          .addStage<data_pipeline::PipelineStage<
              std::unique_ptr<std::vector<double>>, long long>>(calculateSum)
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
      enqueued = inputQueue->enqueue(500000); // Example input size
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
  pipeline.countWaitTime(); // Add function to count wait times

  ASSERT_EQ(results.size(), numElements);
}

TEST_F(DataPipelinePerfTest, PipelineWithLargeDataSingleStage) {
  const int numElements = 20;
  const int timeoutMs = 30000;
  const int checkIntervalMs = 10;

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<long, long long>>(
              [](long length) -> long {
                // Combined operations using predefined functions
                std::unique_ptr<std::vector<double>> randomVec =
                    std::make_unique<std::vector<double>>(
                        generateRandomVector(static_cast<int>(length)));
                std::unique_ptr<std::vector<double>> processedVec =
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
      enqueued = inputQueue->enqueue(500000); // Example input size
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
  pipeline.countWaitTime(); // Add function to count wait times

  std::cout << "Single stage executed " << numElements << " times\n";

  ASSERT_EQ(results.size(), numElements);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}