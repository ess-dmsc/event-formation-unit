#include <atomic>
#include <chrono>
#include <common/testutils/TestBase.h>
#include <efu/DataPipeline.h>
#include <exception>
#include <string>
#include <thread>
#include <numeric>
#include <utility>
#include <vector>

class DataPipelineTest : public TestBase {
protected:
  void SetUp() override {}
};

TEST_F(DataPipelineTest, DifferentInputOutputTypes) {
  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 = pipeline.createNewStage<int, std::string>(
      [](int input) { return std::to_string(input); });
  auto &stage2 = pipeline.createNewStage<std::string, size_t>(
      [](std::string input) { return input.length(); });

  stage1.connectOutput(stage2.getInputQueue());

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage2.getOutputQueue();

  pipeline.start();

  inputQueue->enqueue(123);
  inputQueue->enqueue(4567);
  inputQueue->enqueue(89);

  std::vector<size_t> results;
  size_t result;
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
  EXPECT_EQ(results[0], 3); // "123" has 3 characters
  EXPECT_EQ(results[1], 4); // "4567" has 4 characters
  EXPECT_EQ(results[2], 2); // "89" has 2 characters
}

TEST_F(DataPipelineTest, GracefulStopOnException) {
  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 = pipeline.createNewStage<int, std::string>([](int input) {
    if (input == 100) {
      return std::string("error");
    } else {
      return std::to_string(input);
    }
  });

  auto &stage2 =
      pipeline.createNewStage<std::string, size_t>([](std::string input) {
        if (input == "error") {
          throw std::runtime_error("Test exception");
        }
        return input.length();
      });

  stage1.connectOutput(stage2.getInputQueue());

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage2.getOutputQueue();

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
  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 =
      pipeline.createNewStage<int, int>([](int input) { return input * 2; });

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage1.getOutputQueue();

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
  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 =
      pipeline.createNewStage<int, int>([](int input) { return input * 2; });
  auto &stage2 =
      pipeline.createNewStage<int, int>([](int input) { return input + 1; });

  stage1.connectOutput(stage2.getInputQueue());

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage2.getOutputQueue();

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
  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 = pipeline.createNewStage<int, int>([](int input) {
    if (input == 2) {
      throw std::runtime_error("Test exception");
    }
    return input * 2;
  });

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage1.getOutputQueue();

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
  const int numElements = 1000;
  const int timeoutMs = 10000;
  const int checkIntervalMs = 10;

  data_pipeline::Pipeline pipeline = data_pipeline::Pipeline();

  auto &stage1 = pipeline.createNewStage<long, std::vector<long>>([](long length) {
    std::vector<long> data(length);
    std::generate(data.begin(), data.end(), []() { return rand() % 20 + 1; });
    return data;
  });

  auto &stage2 = pipeline.createNewStage<std::vector<long>, std::vector<long>>([](std::vector<long> input) {
    std::sort(input.begin(), input.end());
    return input;
  });

  auto &stage3 = pipeline.createNewStage<std::vector<long>, long>([](std::vector<long> input) {
    return std::accumulate(input.begin(), input.end(), 0L);
  });

  stage1.connectOutput(stage2.getInputQueue());
  stage2.connectOutput(stage3.getInputQueue());

  auto inputQueue = stage1.getInputQueue();
  auto outputQueue = stage3.getOutputQueue();

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
      enqueued = inputQueue->enqueue(i);
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

  ASSERT_EQ(results.size(), numElements);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}