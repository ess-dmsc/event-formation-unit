// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "Counters.h"
#include "common/utils/EfuUtils.h"
#include "readout/DataEventTypes.h"
#include "readout/TimingEventHandler.h"
#include "gtest/gtest-death-test.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"
#include <chrono>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace efutils;
using namespace chrono;

auto baseTime = high_resolution_clock::now();
auto frequencyPeriodNs = hzToNanoseconds(14).count();

class TDCDataFactory {
private:
  int publishedEventCounter = 0;
  std::vector<std::unique_ptr<TDCDataEvent>> factoryStorage;

public:
  TDCDataEvent &getNextTDC(int delayInMs = 0) {
    // clang-format off
    std::unique_ptr<TDCDataEvent> newDataEvent = std::make_unique<TDCDataEvent>(
        15, publishedEventCounter,
        1000 + publishedEventCounter * frequencyPeriodNs,
        6,
        TimingEventHandler::DEFAULT_FREQUENCY_NS,
        
        baseTime + milliseconds(frequencyPeriodNs * publishedEventCounter + delayInMs)
        );
    // clang-format on

    factoryStorage.push_back(std::move(newDataEvent));
    publishedEventCounter++;

    return *factoryStorage.back();
  }
};

class EVRDataFactory {
private:
  int publishedEventCounter = 0;
  std::vector<std::unique_ptr<EVRDataEvent>> factoryStorage;

public:
  EVRDataEvent &getNextEVR(int delayInMs = 0) {
    // clang-format off
   std::unique_ptr<EVRDataEvent> newDataEvent = std::make_unique<EVRDataEvent>(
        1, 0, 0,
        publishedEventCounter, 
        50000, 50000, 
        40000, 40000,
        baseTime + milliseconds(frequencyPeriodNs * publishedEventCounter +
                                delayInMs)
                                );
    // clang-format on

    factoryStorage.push_back(std::move(newDataEvent));
    publishedEventCounter++;

    return *factoryStorage.back();
  }
};

class Timepix3TimingEventHandlerTest : public TestBase {
protected:
  unique_ptr<Counters> counters;

  unique_ptr<TimingEventHandler> testEventHandler;
  unique_ptr<TDCDataFactory> tdcFactory;
  unique_ptr<EVRDataFactory> evrFactory;

  const uint64_t testTdcTimeStamp =
      TDC_CLOCK_BIN_NS * 31447764897 + TDC_FINE_CLOCK_BIN_NS * 6;

  void SetUp() override {
    counters.reset(new Counters());
    testEventHandler.reset(new TimingEventHandler(*counters));
    evrFactory.reset(new EVRDataFactory());
    tdcFactory.reset(new TDCDataFactory());
  }
  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3TimingEventHandlerTest, FindTDCPair) {

  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(tdcFactory->getNextTDC(10));

  EXPECT_EQ(counters->FoundEVRandTDCPairs, 1);
  EXPECT_EQ(counters->MissEVRPair, 0);
  EXPECT_EQ(counters->MissTDCPair, 0);

  EXPECT_EQ(testEventHandler->getLastTDCPair(),
            testEventHandler->getLastTdcEvent());
}

TEST_F(Timepix3TimingEventHandlerTest, DelayedTDCTest) {

  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(tdcFactory->getNextTDC(40));
  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(tdcFactory->getNextTDC(2));

  EXPECT_EQ(counters->FoundEVRandTDCPairs, 2);
  EXPECT_EQ(counters->MissTDCPair, 1);
  EXPECT_EQ(counters->MissEVRPair, 0);
  EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
            testEventHandler->getLastTDCPair()->counter);
}

TEST_F(Timepix3TimingEventHandlerTest, MissingTDCTest) {
  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(evrFactory->getNextEVR());
  EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
            testEventHandler->getLastTDCPair()->counter);

  // TDC produced but not received
  tdcFactory->getNextTDC();
  testEventHandler->applyData(evrFactory->getNextEVR());
  /// \todo Check this situation when current EVR has no TDC pair
  // EXPECT_EQ(testEventHandler->getLastEvrEvent()->Counter,
  //           testEventHandler->getLastTDCPair()->counter);

  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(evrFactory->getNextEVR());
  EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
            testEventHandler->getLastTDCPair()->counter);

  EXPECT_EQ(counters->FoundEVRandTDCPairs, 2);
  EXPECT_EQ(counters->MissTDCPair, 1);
}

TEST_F(Timepix3TimingEventHandlerTest, ContinousEVRMessages) {

  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(evrFactory->getNextEVR());
  testEventHandler->applyData(evrFactory->getNextEVR());

  EXPECT_EQ(counters->FoundEVRandTDCPairs, 0);
  EXPECT_EQ(counters->MissEVRPair, 0);
  EXPECT_EQ(counters->MissTDCPair, 3);
}

TEST_F(Timepix3TimingEventHandlerTest, ContinousTDCMessages) {

  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(tdcFactory->getNextTDC());
  testEventHandler->applyData(tdcFactory->getNextTDC());

  EXPECT_EQ(counters->FoundEVRandTDCPairs, 0);
  EXPECT_EQ(counters->MissEVRPair, 3);
  EXPECT_EQ(counters->MissTDCPair, 0);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}