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

class Timepix3PixelEventHandlerTest : public TestBase {
protected:
  Counters counters{};
  shared_ptr<Timepix3Geometry> geometry{new Timepix3Geometry(256, 256, 1)};
  MockEV44Serializer serializer;
  PixelEventHandler testEventHandler{counters, geometry, serializer};

  static constexpr int64_t TEST_PULSE_TIME_NS = 1706778348000000240;

  // TDC clock information to calculate TDC pxel time for
  // quarter: 2
  // pixel time: 17s
  static constexpr uint64_t TEST_TDC_TIMESTAMP = 14029934583;
  static constexpr uint64_t TEST_TDC_FINE_CLOCK = 100;
  static constexpr uint64_t TEST_TDC_TIMESTAMP_NS =
      TEST_TDC_TIMESTAMP * TDC_CLOCK_BIN_NS +
      TEST_TDC_FINE_CLOCK * TDC_FINE_CLOCK_BIN_NS;

  static constexpr uint8_t TEST_TDC_QUARTER =
      uint8_t(TEST_TDC_TIMESTAMP_NS / PIXEL_MAX_TIMESTAMP_NS);

  static constexpr uint64_t TEST_TDC_PIXEL_TIME_IN_NS =
      TEST_TDC_TIMESTAMP_NS - (PIXEL_MAX_TIMESTAMP_NS * TEST_TDC_QUARTER);

  void SetUp() override {
    // Recreate initialize test objects to reset their memory
    new (&counters) Counters();
    new (&serializer) MockEV44Serializer();
    new (&testEventHandler) PixelEventHandler(counters, geometry, serializer);

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
  testEventHandler.applyData({TEST_PULSE_TIME_NS, TEST_TDC_PIXEL_TIME_IN_NS});
}

TEST_F(Timepix3PixelEventHandlerTest, TestInvalidPixelReadout) {
  
  testEventHandler.applyData(PixelReadout{15, 2231, 30,
                                                  6, 45, 100, 41503});

  EXPECT_EQ(counters.InvalidPixelReadout, 1);
}


TEST_F(Timepix3PixelEventHandlerTest, TestIfTofIsBiggerThenFrequency) {

  serializer.pulseTimeToCompare = 100000;
  testEventHandler.applyData({100000, 23});
  testEventHandler.applyData(PixelReadout{15, 10, 30,
                                                  6, 10, 100, 41503});

  testEventHandler.pushDataToKafka();
  EXPECT_EQ(counters.EventTimeForNextPulse, 1);
  EXPECT_EQ(serializer.addEventCallCounter, 0);
}

TEST_F(Timepix3PixelEventHandlerTest, TestEventProccesedAndPublished) {

  serializer.pulseTimeToCompare = TEST_PULSE_TIME_NS;
  testEventHandler.applyData({TEST_PULSE_TIME_NS, TEST_TDC_PIXEL_TIME_IN_NS});
  testEventHandler.applyData(PixelReadout{15, 10, 30,
                                                  6, 10, 20000, 41503});

  testEventHandler.pushDataToKafka();
  EXPECT_EQ(counters.EventTimeForNextPulse, 0);
  EXPECT_EQ(serializer.addEventCallCounter, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}