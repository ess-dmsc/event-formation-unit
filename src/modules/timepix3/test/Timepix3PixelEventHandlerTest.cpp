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

  int32_t pulseTimeToCompare;
  int64_t eventTimeToCompare;
  int32_t pixelIdToCompare;

  MOCK_METHOD(void, setReferenceTime, (int64_t referenceTime), (override));
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t pixelId), (override));
};

class Timepix3PixelEventHandlerTest : public TestBase {
protected:
  Counters counters{};
  shared_ptr<Timepix3Geometry> geometry{new Timepix3Geometry(256, 256, 1)};
  MockEV44Serializer serializer;
  PixelEventHandler testEventHandler{counters, geometry, serializer};

  void SetUp() override {
    // Recreate initialize test objects to reset their memory
    new (&counters) Counters();
    new (&testEventHandler) PixelEventHandler(counters, geometry, serializer);

    ON_CALL(serializer, setReferenceTime(testing::_))
        .WillByDefault([this](int64_t referenceTime) {
          EXPECT_EQ(referenceTime, serializer.pulseTimeToCompare);
        });

    ON_CALL(serializer, addEvent(testing::_, testing::_))
        .WillByDefault([this](int32_t pulseTime, int32_t pixelId) -> size_t {
          EXPECT_EQ(pulseTime, serializer.pulseTimeToCompare);
          EXPECT_EQ(pixelId, serializer.pixelIdToCompare);

          return 1;
        });
  }

  void TearDown() override {}
};

// Test cases below

TEST_F(Timepix3PixelEventHandlerTest, SerializerReferenceTimeUpdated) {

  serializer.pulseTimeToCompare = 100000;
  testEventHandler.applyData({100000, 23});
}

TEST_F(Timepix3PixelEventHandlerTest, TestPixelDataProcessing) {

  serializer.pulseTimeToCompare = 100000;
  testEventHandler.applyData(PixelReadout{15, 2231, 30,
                                                  6, 45, 454, 33});

  testEventHandler.pushDataToKafka();
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}