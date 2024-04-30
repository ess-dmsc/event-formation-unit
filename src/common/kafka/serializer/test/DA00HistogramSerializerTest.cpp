// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of Frame1DHistogramSerializer class
///
//===----------------------------------------------------------------------===//

#include <FlatbufferTypes.h>
#include <bits/types/time_t.h>
#include <chrono>
#include <cmath>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/math/NumericalMath.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <exception>
#include <gtest/gtest.h>
#include <ratio>
#include <string>
#include <vector>
#include "common/kafka/Producer.h"
#include "common/time/ESSTime.h"

using namespace std::chrono;
using namespace esstime;

struct CommonTestData {
  std::string Source;
  std::string Name;
  std::string Unit;
  size_t BinSize;
  TimeDurationNano ReferenceTime;
  int64_t ProduceCallTime;

  CommonTestData &setSource(const std::string &source) {
    Source = source;
    return *this;
  }

  CommonTestData &setName(const std::string &name) {
    Name = name;
    return *this;
  }

  CommonTestData &setUnit(const std::string &unit) {
    Unit = unit;
    return *this;
  }

  CommonTestData &setBinSize(size_t binSize) {
    BinSize = binSize;
    return *this;
  }

  CommonTestData &setReferenceTime(TimeDurationNano referenceTime) {
    ReferenceTime = referenceTime;
    return *this;
  }

  CommonTestData &setProduceCallTime(int64_t produceCallTime) {
    ProduceCallTime = produceCallTime;
    return *this;
  }
};

template <typename T> class TestValidator {
private:
  CommonTestData TestData;

public:
  TestValidator(const CommonTestData &testData) : TestData(testData) {}

  void flatbufferTester(nonstd::span<const uint8_t> TestFlatBuffer,
                        int64_t TestProduceCallTime) {
    const uint8_t *FbPtr = TestFlatBuffer.data();

    auto DeserializedDataArray = da00flatbuffers::DataArray(
        flatbuffers::GetRoot<const da00_DataArray>(FbPtr));

    EXPECT_NEAR(TestProduceCallTime, TestData.ProduceCallTime, 10);
    EXPECT_EQ(DeserializedDataArray.getSourceName(), TestData.Source);
    EXPECT_EQ(DeserializedDataArray.getTimeStamp(), TestData.ReferenceTime);
    EXPECT_EQ(DeserializedDataArray.getData().size(), 2);
  }
};

class HistogramSerializerTest : public TestBase {

protected:
  std::string Topic{"test_topic"};
  std::string Source{"test_source"};
  std::string Unit{"millisecond"};

  CommonTestData TestData;

  fbserializer::HistrogramSerializerStats Stats;

  void SetUp() override {
    Stats = fbserializer::HistrogramSerializerStats();
    TestData =
        CommonTestData()
            .setSource(Source)
            .setName("testData")
            .setUnit(Unit)
            .setBinSize(10)
            .setProduceCallTime(duration_cast<milliseconds>(
                                    system_clock::now().time_since_epoch())
                                    .count());
  }

  void TearDown() override {}
};

TEST_F(HistogramSerializerTest, TestIntConstructor) {
  int32_t Period = 1000;
  int32_t Count = 10;

  int64_t ProduceCallTime =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count();

  TimeDurationNano TestRefTime{std::chrono::seconds(10)};

  std::vector<uint64_t> data{0};

  TestData.setProduceCallTime(ProduceCallTime);
  TestData.setReferenceTime(TestRefTime);

  TestValidator<uint64_t> TestValidator = ::TestValidator<uint64_t>(TestData);

  ProducerCallback MockProduceFunction =
      [&TestValidator](nonstd::span<const uint8_t> arg1, int64_t arg2) {
        return TestValidator.flatbufferTester(arg1, arg2);
      };

  auto serializer = fbserializer::HistogramSerializer<uint64_t>(
      Topic, Source, Period, Count, "testData", "adc", "millisecond", Stats,
      MockProduceFunction);

  serializer.setReferenceTime(ESSTime(TestRefTime));

  serializer.produce();
}

// TEST_F(HistogramSerializerTest, TestIntConstructorNegativeValues) {
//   int32_t period = -1000;
//   int32_t count = 10;
//   EXPECT_THROW(HistogramSerializer<uint64_t>("some topic", "source", period,
//                                              count, "testData", "adc", Stats,
//                                              function),
//                std::domain_error);

//   period = 1000;
//   count = -10;
//   EXPECT_THROW(HistogramSerializer<uint64_t>("some topic", "source", period,
//                                              count, "testData", "adc", Stats,
//                                              function),
//                std::domain_error);
// }

// TEST_F(HistogramSerializerTest, TestUint64Uint64Constructor) {
//   int32_t period = 1000000000;
//   int32_t count = 400;
//   auto serializer =
//       HistogramSerializer<uint64_t>("some topic", "source", period, count,
//                                     "testData", "adc", Stats, function);

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(serializer.getInternalXAxis().size(), count);
//   for (size_t i = 0; i < serializer.getInternalXAxis().size(); ++i) {
//     EXPECT_EQ((serializer.getInternalXAxis())[i],
//               i * static_cast<uint64_t>(period) / count);
//   }
// }

// TEST_F(HistogramSerializerTest, TestFloatConstructor) {
//   int32_t period = 100;
//   int32_t count = 30;
//   auto serializer =
//       HistogramSerializer<float>("some topic", "source", period, count,
//                                  "testData", "adc", Stats, function);

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(serializer.getInternalXAxis().size(), count);

//   float step = static_cast<float>(period) / static_cast<float>(count);

//   float value = 0;
//   for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
//     EXPECT_EQ(serializer.getInternalXAxis()[i], value);
//     value += step;
//   }
// }

// TEST_F(HistogramSerializerTest, TestDoubleConstructor) {
//   int32_t period = 1;
//   int32_t count = 30;
//   auto serializer =
//       HistogramSerializer<double>("some topic", "source", period, count,
//                                   "testData", "adc", Stats, function);

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(serializer.getInternalXAxis().size(), count);

//   double step = static_cast<double>(period) / static_cast<double>(count);

//   double value = 0;
//   for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
//     EXPECT_EQ(serializer.getInternalXAxis()[i], value);
//     value += step;
//   }
// }

// TEST_F(HistogramSerializerTest, EDGETestIntConstructorWithFractional) {
//   int32_t period = 1;
//   int32_t count = 30;
//   auto serializer =
//       HistogramSerializer<uint>("some topic", "source", period, count,
//                                 "testData", "adc", Stats, function);

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(serializer.getInternalXAxis().size(), count);

//   uint step = static_cast<uint>(period) / static_cast<uint>(count);

//   uint value = 0;
//   for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
//     EXPECT_EQ(serializer.getInternalXAxis()[i], value);
//     value += step;
//   }
// }

// TEST_F(HistogramSerializerTest, TestIntegerBinning) {
//   int32_t period = 20;
//   int32_t count = 2;
//   auto serializer = fbserializer::HistogramSerializer<int>(
//       "some topic", "source", period, count, "testData", "adc", "milli",
//       Stats, function);

//   std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};

//   for (auto &e : testData) {
//     serializer.addEvent(e.first, e.second);
//   }

//   // EXPECT_EQ(serializer.getInternalData().size(), count);
//   // for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
//   //   EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]),
//   //   1);
//   // }

//   serializer.produce();
// }

// TEST_F(HistogramSerializerTest, TestNegativeIntegerBinning) {
//   int32_t period = 20;
//   int32_t count = 2;
//   auto serializer =
//       HistogramSerializer<int>("some topic", "source", period, count,
//                                "testData", "adc", Stats, function);

//   std::map<int, int> testData = {{-33, 1}, {8, 1}, {-12, 1}, {12, 1}};

//   for (auto &e : testData) {
//     serializer.addEvent(e.first, e.second);
//   }

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
//     EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]),
//     1);
//   }
// }

// TEST_F(HistogramSerializerTest, TestHigherTimeThenPeriodDropped) {
//   int32_t period = 20;
//   int32_t count = 2;
//   auto serializer =
//       HistogramSerializer<int>("some topic", "source", period, count,
//                                "testData", "adc", Stats, function);

//   std::map<int, int> testData = {{3, 1}, {8, 0}, {12, 1}, {25, 1}, {26, 1}};

//   for (auto &e : testData) {
//     serializer.addEvent(e.first, e.second);
//   }

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(Stats.DataOverPeriodDropped, 2);
//   EXPECT_EQ(Stats.DataOverPeriodLastBin, 0);
//   for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
//     EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]),
//     1);
//   }
// }

// TEST_F(HistogramSerializerTest, TestHigherTimeThenPeriodLastBin) {
//   int32_t period = 20;
//   int32_t count = 2;
//   auto serializer = HistogramSerializer<int>(
//       "some topic", "source", period, count, "testData", "adc", Stats,
//       function, fbserializer::BinningStrategy::LastBin);

//   std::map<int, int> testData = {{3, 1},  {8, 1},  {12, 0},
//                                  {20, 0}, {25, 1}, {26, 1}};

//   for (auto &e : testData) {
//     serializer.addEvent(e.first, e.second);
//   }

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   EXPECT_EQ(Stats.DataOverPeriodDropped, 0);
//   EXPECT_EQ(Stats.DataOverPeriodLastBin, 3);
//   for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
//     EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]),
//     2);
//   }
// }

// TEST_F(HistogramSerializerTest, EdgeTestFractionalBinning) {
//   int32_t period = 20;
//   int32_t count = 3;
//   float step = static_cast<float>(period) / static_cast<float>(count);

//   auto serializer =
//       HistogramSerializer<float>("some topic", "source", period, count,
//                                  "testData", "adc", Stats, function);

//   float minFloatStep =
//       std::nextafter(step, std::numeric_limits<float>::max()) - step;

//   std::map<float, int> testData = {
//       {1.0, 0}, {step - minFloatStep, 1}, {step, 1}, {step * 2, 1}};

//   for (auto &e : testData) {
//     serializer.addEvent(e.first, e.second);
//   }

//   EXPECT_EQ(serializer.getInternalData().size(), count);
//   for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
//     std::cout << "Loop status: " << i + 1 << "/"
//               << serializer.getInternalData().size() << std::endl;
//     EXPECT_EQ(essmath::SUM_AGG_FUNC<float>(serializer.getInternalData()[i]),
//     1);
//   }
// }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}