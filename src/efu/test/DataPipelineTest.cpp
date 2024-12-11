// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Tests for the data pipeline implementation
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <efu/DataPipeline.h>
#include <numeric>

class DataPipelineTest : public TestBase {
protected:
  void SetUp() override {}
};

TEST_F(DataPipelineTest, TestBuildThrowsIfNoStages) {
  auto pipelineBuilder = data_pipeline::PipelineBuilder();

  auto builder = data_pipeline::PipelineBuilder();
  builder.addStage<data_pipeline::PipelineStage<float, int>>(
      [](float input) { return input * 2; });
  builder.addStage<data_pipeline::PipelineStage<int, std::string>>(
      [](int input) { return std::to_string(input); });

  EXPECT_THROW(data_pipeline::Pipeline pipeline = builder.build(),
               std::runtime_error);
}

TEST_F(DataPipelineTest, TestPipelineBuilding) {
  auto pipelineBuilder = data_pipeline::PipelineBuilder();

  auto pipeline = pipelineBuilder
                      .addStage<data_pipeline::PipelineStage<float, int>>(
                          [](float input) { return input * 2; })
                      .addStage<data_pipeline::PipelineStage<int, std::string>>(
                          [](int input) { return std::to_string(input); })
                      .build();

  auto inputQueuePtr = pipeline.getInputQueue<float>();
  auto outputQueuePtr = pipeline.getOutputQueue<std::string>();

  pipeline.start();

  inputQueuePtr->enqueue(1.4);

  std::string result = "";
  while (!outputQueuePtr->dequeue(result)) {
    std::this_thread::yield(); // Yield to avoid busy waiting if the queue is
                               // empty
  }

  pipeline.stop();

  ASSERT_EQ(result, "2");
}

TEST_F(DataPipelineTest, DifferentInputOutputTypes) {

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<int, std::string>>(
              [](int input) { return std::to_string(input); })
          .addStage<data_pipeline::PipelineStage<std::string, size_t>>(
              [](std::string input) { return input.length(); })
          .build();

  auto inputQueuePtr = pipeline.getInputQueue<int>();
  auto outputQueuePtr = pipeline.getOutputQueue<size_t>();

  pipeline.start();

  inputQueuePtr->enqueue(123);
  inputQueuePtr->enqueue(4567);
  inputQueuePtr->enqueue(89);

  std::vector<size_t> results;
  size_t result;
  const int expectedResults = 3;
  const int timeoutMs = 1000;
  const int checkIntervalMs = 10;
  int elapsedMs = 0;

  while (results.size() < expectedResults && elapsedMs < timeoutMs) {
    while (outputQueuePtr->dequeue(result)) {
      results.push_back(result);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    elapsedMs += checkIntervalMs;
  }

  pipeline.stop();

  ASSERT_EQ(results.size(), expectedResults);
  EXPECT_EQ(results[0], 3); // "123" has 3 characters
  EXPECT_EQ(results[1], 4); // "4567" has 4 characters
  EXPECT_EQ(results[2], 2); // "89" has 2 characters
}

TEST_F(DataPipelineTest, GracefulStopOnException) {
  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<int, std::string>>(
              [](int input) {
                if (input == 100) {
                  return std::string("error");
                } else {
                  return std::to_string(input);
                }
              })
          .addStage<data_pipeline::PipelineStage<std::string, size_t>>(
              [](std::string input) {
                if (input == "error") {
                  throw std::runtime_error("Test exception");
                }
                return input.length();
              })
          .build();

  auto inputQueue = pipeline.getInputQueue<int>();
  auto outputQueue = pipeline.getOutputQueue<size_t>();

  pipeline.start();

  inputQueue->enqueue(100);
  inputQueue->enqueue(123);
  inputQueue->enqueue(4567);
  inputQueue->enqueue(123);

  const int timeoutMs = 1000;
  const int checkIntervalMs = 10;
  int elapsedMs = 0;

  while (pipeline.isPipelineHealty() && elapsedMs < timeoutMs) {
    std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    elapsedMs += checkIntervalMs;
  }

  std::vector<std::runtime_error> pipelineErrors = pipeline.getPipelineErros();
  pipeline.stop();

  ASSERT_FALSE(pipeline.isPipelineHealty());
  EXPECT_EQ(pipelineErrors.size(), 1);
  EXPECT_STREQ(pipelineErrors.back().what(), "Test exception");
  EXPECT_EQ(pipeline.getRunningStages(), 0);
}

TEST_F(DataPipelineTest, SingleStagePipeline) {
  auto pipeline = data_pipeline::PipelineBuilder()
                      .addStage<data_pipeline::PipelineStage<int, int>>(
                          [](int input) { return input * 2; })
                      .build();

  auto inputQueue = pipeline.getInputQueue<int>();
  auto outputQueue = pipeline.getOutputQueue<int>();

  pipeline.start();

  inputQueue->enqueue(1);
  inputQueue->enqueue(2);
  inputQueue->enqueue(3);

  std::vector<int> results;
  int result;
  const int expectedResults = 3;
  const int timeoutMs = 1000;
  const int checkIntervalMs = 10;
  int elapsedMs = 0;

  while (results.size() < expectedResults && elapsedMs < timeoutMs) {
    while (outputQueue->dequeue(result)) {
      results.push_back(result);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    elapsedMs += checkIntervalMs;
  }

  pipeline.stop();

  ASSERT_EQ(results.size(), expectedResults);
  EXPECT_EQ(results[0], 2); // 1 * 2
  EXPECT_EQ(results[1], 4); // 2 * 2
  EXPECT_EQ(results[2], 6); // 3 * 2
}

TEST_F(DataPipelineTest, MultipleStagesPipeline) {
  data_pipeline::Pipeline pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<int, int>>(
              [](int input) { return input * 2; })
          .addStage<data_pipeline::PipelineStage<int, int>>(
              [](int input) { return input + 1; })
          .build();

  auto inputQueue = pipeline.getInputQueue<int>();
  auto outputQueue = pipeline.getOutputQueue<int>();

  pipeline.start();

  inputQueue->enqueue(1);
  inputQueue->enqueue(2);
  inputQueue->enqueue(3);

  std::vector<int> results;
  int result;
  const int expectedResults = 3;
  const int timeoutMs = 1000;
  const int checkIntervalMs = 10;
  int elapsedMs = 0;

  while (results.size() < expectedResults && elapsedMs < timeoutMs) {
    while (outputQueue->dequeue(result)) {
      results.push_back(result);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    elapsedMs += checkIntervalMs;
  }

  pipeline.stop();

  ASSERT_EQ(results.size(), expectedResults);
  EXPECT_EQ(results[0], 3); // (1 * 2) + 1
  EXPECT_EQ(results[1], 5); // (2 * 2) + 1
  EXPECT_EQ(results[2], 7); // (3 * 2) + 1
}

TEST_F(DataPipelineTest, PipelineWithExceptionHandling) {
  data_pipeline::Pipeline pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<int, int>>([](int input) {
            if (input == 2) {
              throw std::runtime_error("Test exception");
            }
            return input * 2;
          })
          .build();

  auto inputQueue = pipeline.getInputQueue<int>();
  auto outputQueue = pipeline.getOutputQueue<int>();

  pipeline.start();

  inputQueue->enqueue(1);
  inputQueue->enqueue(2);
  inputQueue->enqueue(3);

  const int timeoutMs = 1000;
  const int checkIntervalMs = 10;
  int elapsedMs = 0;

  while (pipeline.isPipelineHealty() && elapsedMs < timeoutMs) {
    std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    elapsedMs += checkIntervalMs;
  }

  std::vector<std::runtime_error> pipelineErrors = pipeline.getPipelineErros();
  pipeline.stop();

  ASSERT_FALSE(pipeline.isPipelineHealty());
  EXPECT_EQ(pipelineErrors.size(), 1);
  EXPECT_STREQ(pipelineErrors.back().what(), "Test exception");
  EXPECT_EQ(pipeline.getRunningStages(), 0);
}

TEST_F(DataPipelineTest, PipelineWithLargeData) {
  const int numElements = 6000;
  const int timeoutMs = 10000;
  const int checkIntervalMs = 10;

  auto pipeline =
      data_pipeline::PipelineBuilder()
          .addStage<data_pipeline::PipelineStage<long, std::vector<long>>>(
              [](long length) {
                std::vector<long> data(length);
                std::generate(data.begin(), data.end(),
                              []() { return rand() % 20 + 1; });
                return data;
              })
          .addStage<data_pipeline::PipelineStage<std::vector<long,
                                                 std::vector<long>>>(
              [](std::vector<long> input) {
                std::sort(input.begin(), input.end());
                return input;
              })
          .addStage<data_pipeline::PipelineStage<std::vector<long>, long>>(
              [](std::vector<long> input) {
                return std::accumulate(input.begin(), input.end(), 0L);
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
        std::this_thread::yield(); // Yield to avoid busy waiting if the
        // is empty
      }
    }

    return results;
  });

  for (int i = 0; i < numElements; ++i) {
    bool enqueued = false;
    while (!enqueued) {
      enqueued = inputQueue->enqueue(i);
      std::this_thread::yield(); // Yield to avoid busy waiting if the queue
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

  ASSERT_EQ(results.size(), numElements);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}