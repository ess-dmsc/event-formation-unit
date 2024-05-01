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
#include <sys/types.h>
#include <vector>
#include "common/kafka/Producer.h"
#include "common/time/ESSTime.h"

using namespace std::chrono;
using namespace esstime;
using namespace da00flatbuffers;

struct CommonFbMemebers {
  std::string Source;
  std::string Name;
  std::string DataUnit;
  std::string TimeUnit;
  int64_t Period;
  size_t BinSize;
  TimeDurationNano ReferenceTime;
  int64_t ProduceCallTime;

  CommonFbMemebers &setSource(const std::string &source) {
    Source = source;
    return *this;
  }

  CommonFbMemebers &setName(const std::string &name) {
    Name = name;
    return *this;
  }

  CommonFbMemebers &setDataUnit(const std::string &dataUnit) {
    DataUnit = dataUnit;
    return *this;
  }

  CommonFbMemebers &setTimeUnit(const std::string &timeUnit) {
    TimeUnit = timeUnit;
    return *this;
  }

  CommonFbMemebers &setBinSize(size_t binSize) {
    BinSize = binSize;
    return *this;
  }

  CommonFbMemebers &setReferenceTime(TimeDurationNano referenceTime) {
    ReferenceTime = referenceTime;
    return *this;
  }

  CommonFbMemebers &setProduceCallTime(int64_t produceCallTime) {
    ProduceCallTime = produceCallTime;
    return *this;
  }

  CommonFbMemebers &setPeriod(int64_t period) {
    Period = period;
    return *this;
  }
};

template <typename T, typename R = T> class TestValidator {
private:
  const CommonFbMemebers &CommonMembers;
  std::vector<T> ExpectedResults;

public:
  ProducerCallback MockedProduceFunction;

  TestValidator(const CommonFbMemebers &testData)
      : CommonMembers(testData),
        MockedProduceFunction([this](nonstd::span<const uint8_t> TestFlatBuffer,
                                     int64_t TestProduceCallTime) {
          this->flatbufferTester(TestFlatBuffer, TestProduceCallTime);
        }) {}

  TestValidator(const CommonFbMemebers &testData,
                const std::vector<T> &expectedResults)
      : TestValidator(testData) {
    ExpectedResults = expectedResults;
  }

  TestValidator<T> &setData(const std::vector<T> &data) {
    ExpectedResults = data;
    return *this;
  }

  fbserializer::HistogramSerializer<T, R>
  createHistogramSerializer(fbserializer::HistrogramSerializerStats Stats) {
    return fbserializer::HistogramSerializer<T, R>(
        "Topic", CommonMembers.Source, CommonMembers.Period,
        CommonMembers.BinSize, CommonMembers.Name, CommonMembers.DataUnit,
        CommonMembers.TimeUnit, Stats, MockedProduceFunction);
  }

  void flatbufferTester(nonstd::span<const uint8_t> TestFlatBuffer,
                        int64_t TestProduceCallTime) {
    const uint8_t *FbPtr = TestFlatBuffer.data();

    auto DeserializedDataArray =
        DataArray(flatbuffers::GetRoot<const da00_DataArray>(FbPtr));

    EXPECT_NEAR(TestProduceCallTime, CommonMembers.ProduceCallTime, 10);
    EXPECT_EQ(DeserializedDataArray.getSourceName(), CommonMembers.Source);
    EXPECT_EQ(DeserializedDataArray.getTimeStamp(),
              CommonMembers.ReferenceTime);
    EXPECT_EQ(DeserializedDataArray.getData().size(), 2);

    Variable Axis = DeserializedDataArray.getData().at(0);
    Variable Data = DeserializedDataArray.getData().at(1);

    EXPECT_EQ(Axis.getName(), "time");
    EXPECT_EQ(Axis.getAxes(), std::vector<std::string>{"t"});
    EXPECT_EQ(Axis.getUnit(), CommonMembers.TimeUnit);
    EXPECT_EQ(Axis.getData().size(), CommonMembers.BinSize * sizeof(R));

    EXPECT_EQ(Data.getName(), CommonMembers.Name);
    EXPECT_EQ(Data.getAxes(), std::vector<std::string>{"a.u."});
    EXPECT_EQ(Data.getUnit(), CommonMembers.DataUnit);
    EXPECT_EQ(Data.getData().size(), CommonMembers.BinSize * sizeof(T));

    // Convert Data vector to type T vector
    std::vector<T> dataVector;
    for (size_t i = 0; i < Data.getData().size(); i += sizeof(T)) {
      const T *valuePtr = reinterpret_cast<const T *>(&Data.getData()[i]);
      dataVector.push_back(*valuePtr);
    }

    EXPECT_EQ(dataVector, ExpectedResults);
  }
};

class HistogramSerializerTest : public TestBase {

protected:
  CommonFbMemebers CommonFbMembers;
  fbserializer::HistrogramSerializerStats Stats;

  void SetUp() override {
    Stats = fbserializer::HistrogramSerializerStats();
    CommonFbMembers =
        CommonFbMemebers()
            .setName("TestData")
            .setSource("TestSource")
            .setDataUnit("v")
            .setTimeUnit("millisecounds")
            .setBinSize(10)
            .setProduceCallTime(duration_cast<milliseconds>(
                                    system_clock::now().time_since_epoch())
                                    .count());
  }

  void TearDown() override {}
};

TEST_F(HistogramSerializerTest, TestIntConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(Stats);

  /// Perform test
  serializer.setReferenceTime(ESSTime(CommonFbMembers.ReferenceTime));
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestUInt64Constructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<uint64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<uint64_t> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(Stats);

  /// Perform test
  serializer.setReferenceTime(ESSTime(CommonFbMembers.ReferenceTime));
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestFloatConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<float> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<float> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(Stats);

  /// Perform test
  serializer.setReferenceTime(ESSTime(CommonFbMembers.ReferenceTime));
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntConstructorNegativeValues) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(-1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t> Validator{CommonFbMembers, ExpectedResultData};

  EXPECT_THROW(Validator.createHistogramSerializer(Stats), std::domain_error);

  CommonFbMembers.setPeriod(1000).setBinSize(-10);

  EXPECT_THROW(Validator.createHistogramSerializer(Stats), std::domain_error);
}

TEST_F(HistogramSerializerTest, TestDoubleConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<double> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(Stats);

  /// Perform test
  serializer.setReferenceTime(ESSTime(CommonFbMembers.ReferenceTime));
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntDoubleConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(Stats);

  /// Perform test
  serializer.setReferenceTime(ESSTime(CommonFbMembers.ReferenceTime));
  serializer.produce();
}

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
//   // EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]),
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

//   std::map<int, int> testData = {{3, 1}, {8, 0}, {12, 1}, {25, 1}, {26,
//   1}};

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