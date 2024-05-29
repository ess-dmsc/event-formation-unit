// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of Frame1DHistogramSerializer class
///
//===----------------------------------------------------------------------===//

#include <FlatbufferTypes.h>
#include <common/kafka/Producer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/math/NumericalMath.h>
#include <common/testutils/TestBase.h>
#include <common/time/ESSTime.h>

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

  void setData(const std::vector<T> &Data) {
    this->ExpectedResults = Data;
  }

  fbserializer::HistogramSerializer<T, R> createHistogramSerializer() {
    return fbserializer::HistogramSerializer<T, R>(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.Name, CommonMembers.DataUnit, CommonMembers.TimeUnit,
        MockedProduceFunction);
  }

  fbserializer::HistogramSerializer<T, R>
  createHistogramSerializer(fbserializer::BinningStrategy Strategy) {
    return fbserializer::HistogramSerializer<T, R>(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.Name, CommonMembers.DataUnit, CommonMembers.TimeUnit,
        Strategy, MockedProduceFunction);
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

    double step = static_cast<double>(CommonMembers.Period) /
                  static_cast<double>(CommonMembers.BinSize);

    std::vector<R> axisVector;
    for (size_t i = 0; i < Axis.getData().size(); i += sizeof(R)) {
      const R *valuePtr = reinterpret_cast<const R *>(&Axis.getData()[i]);
      axisVector.push_back(*valuePtr);
    }

    for (size_t i = 0; i < axisVector.size(); i++) {
      EXPECT_NEAR(axisVector[i], i * step, 0.0001);
    }

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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);
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

  EXPECT_EQ(serializer.getStats().ProduceCalled, 1);
  EXPECT_EQ(serializer.getStats().ProduceFailedNoReferenceTime, 1);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();

  EXPECT_EQ(serializer.getStats().DataOverPeriodDropped, 0);
  EXPECT_EQ(serializer.getStats().DataOverPeriodLastBin, 2);
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
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();

  EXPECT_EQ(serializer.getStats().DataOverPeriodDropped, 0);
  EXPECT_EQ(serializer.getStats().DataOverPeriodLastBin, 0);
}

TEST_F(HistogramSerializerTest, TestReferenceTimeTriggersProduce) {
  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};
  std::vector<int64_t> ExpectedResultData = {1, 1};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  TestValidator<int64_t, double> Validator{CommonFbMembers, {0, 0}};
  auto serializer = Validator.createHistogramSerializer();

  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  Validator.setData(ExpectedResultData);
  /// Perform test
  serializer.setReferenceTime(CommonFbMembers.ReferenceTime);

  EXPECT_EQ(serializer.getStats().ProduceRefTimeTriggered, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}