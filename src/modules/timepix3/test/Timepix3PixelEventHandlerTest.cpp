// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include "common/kafka/EV44Serializer.h"
#include "geometry/Timepix3Geometry.h"
#include "gtest/gtest.h"
#include <common/testutils/TestBase.h>
#include <common/utils/EfuUtils.h>
#include <cstddef>
#include <cstdint>
#include <dto/TimepixDataTypes.h>
#include <gmock/gmock.h>
#include <handlers/PixelEventHandler.h>
#include <memory>
#include <modules/timepix3/Counters.h>

using namespace Timepix3;
using namespace timepixDTO;
using namespace efutils;
using namespace timepixReadout;
using namespace std;

class MockEV44Serializer : public EV44Serializer {
public:
  MockEV44Serializer() : EV44Serializer(0, "dummy", {}) {}

  int64_t pulseTimeToCompare;
  int32_t eventTimeToCompare;
  int32_t pixelIdToCompare;

  int setReferenceTimeCallCounter{0};
  int addEventCallCounter{0};

  MOCK_METHOD(void, setReferenceTime, (int64_t referenceTime), (override));
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t pixelId), (override));
};

// Helper class to calculate pixel time and event time of flight for testing
// By default values are represent a situation where tdc arrives at ~17s in
// pixel time and the spidr time is close as possible to this event.
//
// Manipulate ToA and FToA to test different situations
struct PixelTimeTestHelper {
  const uint16_t ToA;
  const uint8_t fToA;
  const uint16_t ToT{200};
  const uint32_t spidrTime{41503};
  const uint64_t tdcClockInPixelTime{16999999997};
  const uint64_t pixelClockTime;

  PixelTimeTestHelper(int16_t ToA, uint8_t fToA)
      : ToA(ToA), fToA(fToA),
        pixelClockTime(409600 * static_cast<uint64_t>(spidrTime) +
                       25 * static_cast<uint64_t>(ToA) -
                       1.5625 * static_cast<uint64_t>(fToA)) {}

  uint32_t getEventTof() const { return pixelClockTime - tdcClockInPixelTime; }
};

// Default pixel time 1ns after the tdc clock
static const PixelTimeTestHelper TEST_DEFAULT_PIXEL_TIME{14848, 1};

class Timepix3PixelEventHandlerTest : public TestBase {
protected:
  static constexpr int64_t TEST_PULSE_TIME_NS = 1706778348000000000;
  static constexpr uint64_t TEST_X_COORD = 35;
  static constexpr uint64_t TEST_Y_COORD = 115;
  static constexpr uint8_t TEST_PIX = 8;

  Counters counters{};
  shared_ptr<Timepix3Geometry> geometry{new Timepix3Geometry(256, 256, 1)};
  MockEV44Serializer serializer;
  PixelEventHandler testEventHandler{counters, geometry, serializer};

  // Setup test pixel information, use geometry to calculate pixel id. Geometry
  // separately tested.
  const uint32_t TEST_PIXEL_ID = geometry->pixel2D(TEST_X_COORD, TEST_Y_COORD);
  const uint16_t TEST_DCOL = TEST_X_COORD - (TEST_PIX / 4);
  const uint16_t TEST_SPIX = TEST_Y_COORD - (TEST_PIX & 0x3);

  void SetUp() override {
    // Recreate initialize test objects to reset their
    // memoryTEST_TDC_PIXEL_TIME_IN_NSy, serializer);

    ON_CALL(serializer, setReferenceTime(testing::_))
        .WillByDefault([this](int64_t referenceTime) {
          serializer.setReferenceTimeCallCounter++;
          EXPECT_EQ(referenceTime, serializer.pulseTimeToCompare);
        });

    ON_CALL(serializer, addEvent(testing::_, testing::_))
        .WillByDefault([this](int32_t pulseTime, int32_t pixelId) -> size_t {
          serializer.addEventCallCounter++;
          EXPECT_EQ(pulseTime, serializer.eventTimeToCompare);
          EXPECT_EQ(pixelId, serializer.pixelIdToCompare);

          return 1;
        });
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3PixelEventHandlerTest, SerializerReferenceTimeUpdated) {

  serializer.pulseTimeToCompare = TEST_PULSE_TIME_NS;
  testEventHandler.applyData(
      {TEST_PULSE_TIME_NS, TEST_DEFAULT_PIXEL_TIME.tdcClockInPixelTime});
}

TEST_F(Timepix3PixelEventHandlerTest, TestInvalidPixelReadout) {

  uint16_t INVALID_SPIX = 500;
  testEventHandler.applyData(PixelReadout{
      TEST_DCOL, INVALID_SPIX, TEST_PIX, TEST_DEFAULT_PIXEL_TIME.ToT,
      TEST_DEFAULT_PIXEL_TIME.fToA, TEST_DEFAULT_PIXEL_TIME.ToA,
      TEST_DEFAULT_PIXEL_TIME.spidrTime});

  EXPECT_EQ(counters.InvalidPixelReadout, 1);
}

TEST_F(Timepix3PixelEventHandlerTest, TestIfTofIsBiggerThenFrequency) {

  // This value ensures that the pixel time is later then the tdc clock
  uint32_t spidrLateArrival = 55000;

  serializer.pulseTimeToCompare = TEST_PULSE_TIME_NS;

  testEventHandler.applyData(
      {TEST_PULSE_TIME_NS, TEST_DEFAULT_PIXEL_TIME.tdcClockInPixelTime});

  testEventHandler.applyData(
      PixelReadout{TEST_DCOL, TEST_SPIX, TEST_PIX, TEST_DEFAULT_PIXEL_TIME.ToT,
                   TEST_DEFAULT_PIXEL_TIME.fToA, TEST_DEFAULT_PIXEL_TIME.ToA,
                   spidrLateArrival});

  testEventHandler.pushDataToKafka();
  EXPECT_EQ(counters.EventTimeForNextPulse, 1);
  EXPECT_EQ(serializer.addEventCallCounter, 0);
}

TEST_F(Timepix3PixelEventHandlerTest, TestEventProccesedAndPublished) {

  serializer.pulseTimeToCompare = TEST_PULSE_TIME_NS;
  serializer.pixelIdToCompare = TEST_PIXEL_ID;
  serializer.eventTimeToCompare = TEST_DEFAULT_PIXEL_TIME.getEventTof();

  testEventHandler.applyData(
      {TEST_PULSE_TIME_NS, TEST_DEFAULT_PIXEL_TIME.tdcClockInPixelTime});
  testEventHandler.applyData(
      PixelReadout{TEST_DCOL, TEST_SPIX, TEST_PIX, TEST_DEFAULT_PIXEL_TIME.ToT,
                   TEST_DEFAULT_PIXEL_TIME.fToA, TEST_DEFAULT_PIXEL_TIME.ToA,
                   TEST_DEFAULT_PIXEL_TIME.spidrTime});

  testEventHandler.pushDataToKafka();
  EXPECT_EQ(counters.EventTimeForNextPulse, 0);
  EXPECT_EQ(counters.TofCount, 1);
  EXPECT_EQ(counters.PixelErrors, 0);
  EXPECT_EQ(counters.Events, 1);
  EXPECT_EQ(serializer.addEventCallCounter, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}