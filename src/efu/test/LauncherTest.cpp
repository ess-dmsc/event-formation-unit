// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Test for the Launcher class
///
//===----------------------------------------------------------------------===//

#include <chrono>
#include <common/detector/Detector.h>
#include <common/testutils/TestBase.h>
#include <efu/Launcher.h>
#include <memory>
#include <stdexcept>
#include <type_traits>

/// Global counter to keep track of the number of threads that have finished
static std::atomic<int> ThreadFinishedCounter = 0;

class LauncherTest : public TestBase {
protected:
  int keep_running = 1;

  std::shared_ptr<Detector> detector;
  int number_of_threads = 5;
  Launcher launcher = Launcher(keep_running);

  void SetUp() override {
    detector = std::make_shared<Detector>(BaseSettings());

    // Add some threads to the detector
    for (int i = 0; i < number_of_threads; ++i) {
      detector->GetThreadInfo().emplace_back(ThreadInfo{
          []() {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            ThreadFinishedCounter++;
          },
          "TestThread" + std::to_string(i), std::thread()});
    }
  }

  void TearDown() override { ThreadFinishedCounter = 0; }
};

TEST_F(LauncherTest, MultipleThreadsLaunchedSucessfully) {
  ASSERT_EQ(ThreadFinishedCounter, 0);
  EXPECT_EQ(detector->GetThreadInfo().size(), number_of_threads);

  launcher.launchThreads(detector);

  // Wait for all threads to finish
  for (auto &ThreadInfo : detector->GetThreadInfo()) {
    ThreadInfo.thread.join();
  }

  EXPECT_EQ(ThreadFinishedCounter, number_of_threads);
  // Check that the keep_running flag was not changed
  EXPECT_EQ(keep_running, 1);
}

TEST_F(LauncherTest, MutipleThreadsOneThrowsExceptionHandled) {

  ASSERT_EQ(ThreadFinishedCounter, 0);
  EXPECT_EQ(detector->GetThreadInfo().size(), number_of_threads);

  // Add a thread that throws an exception
  detector->GetThreadInfo().emplace_back(
      ThreadInfo{[]() {
                   std::this_thread::sleep_for(std::chrono::milliseconds(400));
                   throw std::runtime_error("Test exception");
                 },
                 "TestThread", std::thread()});

  // Check new thread has been added
  ASSERT_EQ(detector->GetThreadInfo().size(), number_of_threads + 1);

  launcher.launchThreads(detector);

  // Wait for all threads to finish
  for (auto &ThreadInfo : detector->GetThreadInfo()) {
    ThreadInfo.thread.join();
  }

  EXPECT_EQ(ThreadFinishedCounter, number_of_threads);
  // Check that the exception was caught and the keep_running flag was set to 0
  EXPECT_EQ(keep_running, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}