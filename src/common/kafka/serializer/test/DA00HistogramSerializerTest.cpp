// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of Frame1DHistogramSerializer class
///
//===----------------------------------------------------------------------===//

#include <common/kafka/Producer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/kafka/serializer/FlatbufferTypes.h>
#include <common/math/NumericalMath.h>
#include <common/testutils/TestBase.h>
#include <common/time/ESSTime.h>
#include <da00_dataarray_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <fmt/core.h>
#include <unistd.h>

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
  int64_t BinOffset{0}; // Add BinOffset with default value

  CommonFbMemebers &setSource(const std::string &Source) {
    this->Source = Source;
    return *this;
  }

  CommonFbMemebers &setName(const std::string &Name) {
    this->Name = Name;
    return *this;
  }

  CommonFbMemebers &setDataUnit(const std::string &DataUnit) {
    this->DataUnit = DataUnit;
    return *this;
  }

  CommonFbMemebers &setTimeUnit(const std::string &TimeUnit) {
    this->TimeUnit = TimeUnit;
    return *this;
  }

  CommonFbMemebers &setBinSize(size_t BinSize) {
    this->BinSize = BinSize;
    return *this;
  }

  CommonFbMemebers &setReferenceTime(TimeDurationNano ReferenceTime) {
    this->ReferenceTime = ReferenceTime;
    return *this;
  }

  CommonFbMemebers &setProduceCallTime(int64_t ProduceCallTime) {
    this->ProduceCallTime = ProduceCallTime;
    return *this;
  }

  CommonFbMemebers &setPeriod(int64_t Period) {
    this->Period = Period;
    return *this;
  }

  CommonFbMemebers &setBinOffset(int64_t Offset) {
    this->BinOffset = Offset;
    return *this;
  }
};

template <typename T, typename R = T> class TestValidator {
private:
  const CommonFbMemebers &CommonMembers;
  std::vector<T> ExpectedResults;

public:
  ProducerCallback MockedProduceFunction;

  TestValidator(const CommonFbMemebers &TestData)
      : CommonMembers(TestData),
        MockedProduceFunction([this](nonstd::span<const uint8_t> TestFlatBuffer,
                                     int64_t TestProduceCallTime) {
          this->flatbufferTester(TestFlatBuffer, TestProduceCallTime);
        }) {}

  TestValidator(const CommonFbMemebers &TestData,
                const std::vector<T> &ExpectedResults)
      : TestValidator(TestData) {
    this->ExpectedResults = ExpectedResults;
  }

  void setData(const std::vector<T> &Data) { this->ExpectedResults = Data; }

  fbserializer::HistogramSerializer<T, R> createHistogramSerializer() {
    return fbserializer::HistogramSerializer<T, R>(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.Name, CommonMembers.DataUnit, CommonMembers.TimeUnit,
        MockedProduceFunction, CommonMembers.BinOffset);
  }

  fbserializer::HistogramSerializer<T, R>
  createHistogramSerializer(fbserializer::BinningStrategy Strategy) {
    return fbserializer::HistogramSerializer<T, R>(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.Name, CommonMembers.DataUnit, CommonMembers.TimeUnit,
        Strategy, MockedProduceFunction, CommonMembers.BinOffset);
  }

  void flatbufferTester(nonstd::span<const uint8_t> TestFlatBuffer,
                        int64_t TestProduceCallTime) {
    const uint8_t *FbPtr = TestFlatBuffer.data();

    EXPECT_TRUE(da00_DataArrayBufferHasIdentifier(FbPtr));

    flatbuffers::Verifier Verifier(FbPtr, TestFlatBuffer.size());
    EXPECT_TRUE(Verifyda00_DataArrayBuffer(Verifier));

    auto DeserializedDataArray =
        DataArray(flatbuffers::GetRoot<const da00_DataArray>(FbPtr));

    EXPECT_NEAR(TestProduceCallTime, CommonMembers.ProduceCallTime, 10);
    EXPECT_EQ(DeserializedDataArray.getSourceName(), CommonMembers.Source);
    EXPECT_EQ(DeserializedDataArray.getTimeStamp(),
              CommonMembers.ReferenceTime);
    EXPECT_EQ(DeserializedDataArray.getData().size(), 2);

    Variable TimeAxis = DeserializedDataArray.getData().at(0);
    Variable SignalAxis = DeserializedDataArray.getData().at(1);

    EXPECT_EQ(TimeAxis.getName(), "time");
    EXPECT_EQ(TimeAxis.getAxes(), std::vector<std::string>{"t"});
    EXPECT_EQ(TimeAxis.getUnit(), CommonMembers.TimeUnit);
    EXPECT_EQ(TimeAxis.getData().size(),
              (CommonMembers.BinSize + 1) * sizeof(R));

    double step =
        (static_cast<double>(CommonMembers.Period) - CommonMembers.BinOffset) /
        static_cast<double>(CommonMembers.BinSize);

    std::vector<R> axisVector;
    for (size_t i = 0; i < TimeAxis.getData().size(); i += sizeof(R)) {
      const R *valuePtr = reinterpret_cast<const R *>(&TimeAxis.getData()[i]);
      axisVector.push_back(*valuePtr);
    }

    for (size_t i = 0; i < axisVector.size(); i++) {
      EXPECT_NEAR(axisVector[i], CommonMembers.BinOffset + i * step, 0.0001);
    }

    EXPECT_EQ(SignalAxis.getName(), CommonMembers.Name);
    EXPECT_EQ(SignalAxis.getAxes(), std::vector<std::string>{"a.u."});
    EXPECT_EQ(SignalAxis.getUnit(), CommonMembers.DataUnit);
    EXPECT_EQ(SignalAxis.getData().size(), CommonMembers.BinSize * sizeof(T));

    // Convert Data vector to type T vector
    std::vector<T> dataVector;
    for (size_t i = 0; i < SignalAxis.getData().size(); i += sizeof(T)) {
      const T *valuePtr = reinterpret_cast<const T *>(&SignalAxis.getData()[i]);
      dataVector.push_back(*valuePtr);
    }

    EXPECT_EQ(dataVector, ExpectedResults);
  }
};

class HistogramSerializerTest : public TestBase {

protected:
  CommonFbMemebers CommonFbMembers;

  void SetUp() override {
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
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestUInt64Constructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<uint64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<uint64_t> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestFloatConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<float> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<float> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntConstructorNegativeValues) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(-1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t> Validator{CommonFbMembers, ExpectedResultData};

  EXPECT_THROW(Validator.createHistogramSerializer(), std::domain_error);

  CommonFbMembers.setPeriod(1000).setBinSize(-10);

  EXPECT_THROW(Validator.createHistogramSerializer(), std::domain_error);
}

TEST_F(HistogramSerializerTest, TestDoubleConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<double> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntDoubleConstructor) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(100).setBinSize(33).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestProduceFailsIfNoReference) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<int64_t> ExpectedResultData(CommonFbMembers.BinSize, 0);

  /// Initialize validator and create serializer
  TestValidator<int64_t> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  serializer.produce();

  EXPECT_EQ(serializer.stats().ProduceCalled, 1);
  EXPECT_EQ(serializer.stats().ProduceFailedNoReferenceTime, 1);
}

TEST_F(HistogramSerializerTest, TestIntegerBinning) {
  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};
  std::vector<int64_t> ExpectedResultData = {1, 1};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestNegativeIntegerBinning) {
  std::map<int, int> testData = {{-33, 1}, {8, 1}, {-12, 1}, {12, 1}};
  std::vector<int64_t> ExpectedResultData = {1, 1};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestHigherTimeThenPeriodDropped) {
  std::map<int, int> testData = {{3, 1}, {8, 0}, {12, 1}, {25, 1}, {26, 1}};
  std::vector<int64_t> ExpectedResultData = {1, 3};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();

  EXPECT_EQ(serializer.stats().DataOverPeriodDropped, 0);
  EXPECT_EQ(serializer.stats().DataOverPeriodLastBin, 2);
}

TEST_F(HistogramSerializerTest, EdgeTestFractionalBinning) {
  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  float step =
      static_cast<float>(CommonFbMembers.Period) / CommonFbMembers.BinSize;

  float minFloatStep =
      std::nextafter(step, std::numeric_limits<float>::max()) - step;
  std::map<float, int> testData = {
      {1.0, 1}, {step - minFloatStep, 1}, {step, 1}, {step + 1, 1}};

  std::vector<int64_t> ExpectedResultData = {2, 2};

  /// Initialize validator and create serializer
  TestValidator<int64_t, float> Validator{CommonFbMembers, ExpectedResultData};
  auto serializer = Validator.createHistogramSerializer();

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();

  EXPECT_EQ(serializer.stats().DataOverPeriodDropped, 0);
  EXPECT_EQ(serializer.stats().DataOverPeriodLastBin, 0);
}

TEST_F(HistogramSerializerTest, TestEmtpyCalls) {

  /// Initialize validator and create serializer
  TestValidator<int64_t, double> Validator{CommonFbMembers,
                                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  EXPECT_NO_THROW(serializer.produce());

  EXPECT_NO_THROW(serializer.produce());

  EXPECT_NO_THROW(serializer.produce());
}

TEST_F(HistogramSerializerTest, TestReferenceTimeTriggersProduce) {
  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};
  std::vector<int64_t> ExpectedResultData = {1, 1};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  TestValidator<int64_t, double> Validator{CommonFbMembers, {0, 0}};
  auto serializer = Validator.createHistogramSerializer();

  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  Validator.setData(ExpectedResultData);
  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime +
                                      TimeDurationNano(10));

  EXPECT_EQ(serializer.stats().ProduceRefTimeTriggered, 1);
}

TEST_F(HistogramSerializerTest, TestBinOffset) {
  /// Setup test condition
  CommonFbMembers.setPeriod(100)
      .setBinSize(10)
      .setReferenceTime(std::chrono::seconds(10))
      .setBinOffset(50); // Set BinOffset

  std::vector<int64_t> ExpectedData(CommonFbMembers.BinSize, 0);
  ExpectedData[1] = 2; // Events at 55 and 59

  // Initialize validator and create serializer using
  // createHistogramSerializer()
  TestValidator<int64_t> Validator{CommonFbMembers, ExpectedData};
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  // Add events before and after the BinOffset
  serializer.addEvent(25, 1);  // Should be dropped due to BinOffset
  serializer.addEvent(55, 1);  // Should go into second bin
  serializer.addEvent(59, 1);  // Should go into second bin
  serializer.addEvent(150, 1); // Should be drtopped due to period

  serializer.produce();

  EXPECT_EQ(serializer.stats().DataOverPeriodDropped,
            1); // One event before offset
  EXPECT_EQ(serializer.stats().DataOverPeriodLastBin, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}