// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "Counters.h"
#include "common/utils/EfuUtils.h"
#include "readout/TimepixDataTypes.h"
#include "readout/TimingEventHandler.h"
#include "test/TimepixTestHelper.h"
#include "gtest/gtest-death-test.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"
#include <chrono>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <timepix3/readout/DataParser.h>

using namespace Timepix3;
using namespace timepixDTO;
using namespace efutils;
using namespace chrono;
using namespace timepixReadout;
using namespace std;

DataEventTestHandler<timepixDTO::ESSGlobalTimeStamp> essGlobalTimeStampHandler;

class Timepix3TimingEventHandlerTest : public TestBase {
protected:
  static constexpr uint64_t TEST_PULSE_TIME = 1706533766;
  static constexpr uint64_t TEST_PULSE_TIME_NS = 970000;
  static constexpr uint64_t TEST_EPOCH_PULSE_TIME =
      TEST_PULSE_TIME * 1e9 + TEST_PULSE_TIME_NS;
  static constexpr uint64_t TEST_TDC_QUARTER = 2;
  static constexpr uint64_t TEST_TDC_BASE_TIME = 13;
  static constexpr uint64_t TEST_TDC_FINE_CLOCK = 6;
  static constexpr uint64_t TEST_TDC_IN_PIXEL_TIME =
      TEST_TDC_BASE_TIME * TDC_CLOCK_BIN_NS +
      TEST_TDC_FINE_CLOCK * TDC_FINE_CLOCK_BIN_NS;
  static constexpr uint64_t TEST_TDC_TIMESTAMP =
      PIXEL_MAX_TIMESTAMP_NS * 2 / TDC_CLOCK_BIN_NS + TEST_TDC_BASE_TIME;

  unique_ptr<Counters> counters;
  unique_ptr<TimingEventHandler> testEventHandler;

  const uint64_t testTdcTimeStamp =
      TDC_CLOCK_BIN_NS * 31447764897 + TDC_FINE_CLOCK_BIN_NS * 6;

  void SetUp() override {
    counters = make_unique<Counters>(1);
    testEventHandler = make_unique<TimingEventHandler>(*counters);
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3TimingEventHandlerTest, FindEVRPair) {

  essGlobalTimeStampHandler.setData(
      ESSGlobalTimeStamp(TEST_EPOCH_PULSE_TIME, TEST_TDC_IN_PIXEL_TIME));
  testEventHandler->subscribe(&essGlobalTimeStampHandler);

  // clang-format off
  EVRReadout evrReadout = EVRReadout(1,
                                     0,
                                     0,
                                     10,
                                     TEST_PULSE_TIME,
                                     TEST_PULSE_TIME_NS,
                                     0,
                                     0);

  TDCReadout tdcReadout = TDCReadout(15, 10, TEST_TDC_TIMESTAMP, TEST_TDC_FINE_CLOCK);
  // clang-format on

  testEventHandler->applyData(evrReadout);
  testEventHandler->applyData(tdcReadout);

  EXPECT_EQ(counters->EVRPairFound, 1);
  EXPECT_EQ(counters->TDCPairFound, 0);
  EXPECT_EQ(counters->MissEVRCounter, 0);
  EXPECT_EQ(counters->MissTDCCounter, 0);
}

// TEST_F(Timepix3TimingEventHandlerTest, DelayedTDCTest) {

//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(tdcFactory->getNextTDC(40));
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(tdcFactory->getNextTDC(2));

//   EXPECT_EQ(counters->FoundEVRandTDCPairs, 2);
//   EXPECT_EQ(counters->MissTDCPair, 1);
//   EXPECT_EQ(counters->MissEVRPair, 0);
//   EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
//             testEventHandler->getLastTDCPair()->counter);
// }

// TEST_F(Timepix3TimingEventHandlerTest, MissingTDCTest) {
//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
//             testEventHandler->getLastTDCPair()->counter);

//   // TDC produced but not received
//   tdcFactory->getNextTDC();
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   /// \todo Check this situation when current EVR has no TDC pair
//   // EXPECT_EQ(testEventHandler->getLastEvrEvent()->Counter,
//   //           testEventHandler->getLastTDCPair()->counter);

//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   EXPECT_EQ(testEventHandler->getLastEvrEvent()->counter,
//             testEventHandler->getLastTDCPair()->counter);

//   EXPECT_EQ(counters->FoundEVRandTDCPairs, 2);
//   EXPECT_EQ(counters->MissTDCPair, 1);
// }

// TEST_F(Timepix3TimingEventHandlerTest, ContinousEVRMessages) {

//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(evrFactory->getNextEVR());
//   testEventHandler->applyData(evrFactory->getNextEVR());

//   EXPECT_EQ(counters->FoundEVRandTDCPairs, 0);
//   EXPECT_EQ(counters->MissEVRPair, 0);
//   EXPECT_EQ(counters->MissTDCPair, 3);
// }

// TEST_F(Timepix3TimingEventHandlerTest, ContinousTDCMessages) {

//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(tdcFactory->getNextTDC());
//   testEventHandler->applyData(tdcFactory->getNextTDC());

//   EXPECT_EQ(counters->FoundEVRandTDCPairs, 0);
//   EXPECT_EQ(counters->MissEVRPair, 3);
//   EXPECT_EQ(counters->MissTDCPair, 0);
// }

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}