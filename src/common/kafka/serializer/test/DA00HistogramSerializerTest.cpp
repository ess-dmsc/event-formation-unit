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
  std::string DataUnit;
  uint8_t AggregatedFrames{1};
  int64_t Period;
  size_t BinSize;
  TimeDurationNano ReferenceTime;
  int64_t ProduceCallTime;
  int64_t BinOffset{0}; // Add BinOffset with default value
 
  CommonFbMemebers &setSource(const std::string &Source) {
    this->Source = Source;
    return *this;
  }

  CommonFbMemebers &setDataUnit(const std::string &DataUnit) {
    this->DataUnit = DataUnit;
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

template <typename T, typename R, typename V> class TestValidator {
public:
  using Serializer_t = fbserializer::HistogramSerializer<T, R, V>;
  using ResultData_t = T;
  using ReferenceTime_t = R;
  using SumValue_t = V;

private:
  const CommonFbMemebers &CommonMembers;
  std::vector<T> ExpectedDataResults{};
  std::vector<ReferenceTime_t> ExpectedReferenceResults{};
  std::vector<SumValue_t> ExpectedIntensityResults{};

public:
  ProducerCallback MockedProduceFunction;
  std::function<void(bool)> Invoked;

  TestValidator(const CommonFbMemebers &TestData)
      : CommonMembers(TestData),
        MockedProduceFunction([this](nonstd::span<const uint8_t> TestFlatBuffer,
                                     int64_t TestProduceCallTime) {
          this->flatbufferTester(TestFlatBuffer, TestProduceCallTime);
        }) {}

  // minor change to constructor above. This one can be used to verify that produce callback
  // has been invoked
  TestValidator(const CommonFbMemebers &TestData, const std::function<void(bool)> &invoked)
      : CommonMembers(TestData),
        MockedProduceFunction([this](nonstd::span<const uint8_t> TestFlatBuffer,
                                     int64_t TestProduceCallTime) {
          this->flatbufferTester(TestFlatBuffer, TestProduceCallTime);
          Invoked(true);
        }), Invoked(invoked) {}

  // TestValidator(const CommonFbMemebers &TestData,
  //               const std::vector<T> &ExpectedDataResults)
  //     : TestValidator(TestData) {
  //   this->ExpectedDataResults = ExpectedDataResults;
  // }

  TestValidator(const CommonFbMemebers &TestData,
                const std::vector<T> &ExpectedDataResults,
                const std::vector<ReferenceTime_t> &ExpectedReferenceResults,
                const std::vector<SumValue_t> &ExpectedIntensityResults)
      : TestValidator(TestData) {
    this->ExpectedDataResults = ExpectedDataResults;
    this->ExpectedReferenceResults = ExpectedReferenceResults;
    this->ExpectedIntensityResults = ExpectedIntensityResults;
  }

  void setData(const std::vector<T> &Data) { this->ExpectedDataResults = Data; }
  void setReference(const std::vector<ReferenceTime_t> &Data) { this->ExpectedReferenceResults = Data; }
  void setIntensity(const std::vector<SumValue_t> &Data) { this->ExpectedIntensityResults = Data; }

  Serializer_t createHistogramSerializer() {
    return Serializer_t(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.DataUnit, CommonMembers.AggregatedFrames, 
        MockedProduceFunction, CommonMembers.BinOffset);
  }

  Serializer_t
  createHistogramSerializer(fbserializer::BinningStrategy Strategy) {
    return Serializer_t(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.DataUnit, Strategy, CommonMembers.AggregatedFrames, MockedProduceFunction,
        CommonMembers.BinOffset);
  }

  Serializer_t
  createHistogramSerializer(fbserializer::BinningStrategy Strategy, 
    essmath::VectorAggregationFunc<T> AggFunc) {
    return Serializer_t(
        CommonMembers.Source, CommonMembers.Period, CommonMembers.BinSize,
        CommonMembers.DataUnit, Strategy, CommonMembers.AggregatedFrames, MockedProduceFunction,
        CommonMembers.BinOffset, AggFunc);
  }
  
  template <typename Type_t>
  std::vector<Type_t> GetSerializedData(const Variable &buffer) {
    std::vector<Type_t> vector;
    for (size_t i = 0; i < buffer.getData().size(); i += sizeof(Type_t)) {
      const Type_t *value = reinterpret_cast<const Type_t *>(&buffer.getData()[i]);
      vector.push_back(*value);
    }
    return vector;
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
    EXPECT_EQ(DeserializedDataArray.getData().size(), 4);

    Variable TimeAxis = DeserializedDataArray.getData().at(0);
    Variable SignalAxis = DeserializedDataArray.getData().at(1);
    Variable ReferenceAxis = DeserializedDataArray.getData().at(2);
    Variable IntensityAxis = DeserializedDataArray.getData().at(3);

    EXPECT_EQ(TimeAxis.getName(), "frame_time");
    EXPECT_EQ(TimeAxis.getAxes(), std::vector<std::string>{"frame_time"});
    EXPECT_EQ(TimeAxis.getUnit(), "ns");
    EXPECT_EQ(TimeAxis.getData().size(),
              (CommonMembers.BinSize + 1) * sizeof(R));

    double step =
        (static_cast<double>(CommonMembers.Period) - CommonMembers.BinOffset) /
        static_cast<double>(CommonMembers.BinSize);

    std::vector<R> axisVector = GetSerializedData<R>(TimeAxis);
    for (size_t i = 0; i < axisVector.size(); i++) {
      EXPECT_NEAR(axisVector[i], CommonMembers.BinOffset + i * step, 0.0001);
    }

    EXPECT_EQ(SignalAxis.getName(), "signal");
    EXPECT_EQ(SignalAxis.getAxes(), std::vector<std::string>{"frame_time"});
    EXPECT_EQ(SignalAxis.getUnit(), CommonMembers.DataUnit);
    EXPECT_EQ(SignalAxis.getData().size(), CommonMembers.BinSize * sizeof(T));

    // Convert Data vector to type T vector
    std::vector<T> dataVector = GetSerializedData<T>(SignalAxis);
    EXPECT_EQ(dataVector, ExpectedDataResults);

    //Get reference time vector 
    EXPECT_EQ(ReferenceAxis.getName(), "reference_time");
    EXPECT_EQ(ReferenceAxis.getAxes(), std::vector<std::string>{"reference_time"});
    EXPECT_EQ(ReferenceAxis.getUnit(), "ns");
    EXPECT_EQ(ReferenceAxis.getData().size(), CommonMembers.AggregatedFrames * sizeof(R));
    std::vector<ReferenceTime_t> referenceVector = GetSerializedData<ReferenceTime_t>(ReferenceAxis);
    EXPECT_EQ(referenceVector, ExpectedReferenceResults);

    //Get intensity vector
    EXPECT_EQ(IntensityAxis.getName(), "frame_total");
    EXPECT_EQ(IntensityAxis.getAxes(), std::vector<std::string>{"reference_time"});
    EXPECT_EQ(IntensityAxis.getUnit(), "counts");
    EXPECT_EQ(IntensityAxis.getData().size(), CommonMembers.AggregatedFrames * sizeof(V));
    std::vector<SumValue_t> intensityVector = GetSerializedData<SumValue_t>(IntensityAxis);
    EXPECT_EQ(intensityVector, ExpectedIntensityResults);
  }
};

class HistogramSerializerTest : public TestBase {

protected:
  CommonFbMemebers CommonFbMembers;

  void SetUp() override {
    CommonFbMembers =
        CommonFbMemebers()
            .setSource("TestSource")
            .setDataUnit("V")
            .setBinSize(10)
            .setProduceCallTime(duration_cast<milliseconds>(
                                    system_clock::now().time_since_epoch())
                                    .count());
  }

  void TearDown() override {}
};

TEST_F(HistogramSerializerTest, TestIntConstructor) {
  using Validator_t = TestValidator<int64_t, int64_t, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestUInt64Constructor) {
  using Validator_t = TestValidator<uint64_t, uint64_t, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestFloatConstructor) {
  using Validator_t = TestValidator<float, float, double>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntConstructorNegativeValues) {
  using Validator_t = TestValidator<int64_t, int64_t, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(-1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  EXPECT_THROW(Validator.createHistogramSerializer(), std::domain_error);

  CommonFbMembers.setPeriod(1000).setBinSize(-10);

  EXPECT_THROW(Validator.createHistogramSerializer(), std::domain_error);
}

TEST_F(HistogramSerializerTest, TestDoubleConstructor) {
  using Validator_t = TestValidator<double, double, double>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestIntDoubleConstructor) {
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(100).setBinSize(33).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);
  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestProduceFailsIfNoReference) {
  using Validator_t = TestValidator<int64_t, int64_t, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(1000).setBinSize(10).setReferenceTime(
      std::chrono::seconds(10));

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {0};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  serializer.produce();

  EXPECT_EQ(serializer.stats().ProduceCalled, 1);
  EXPECT_EQ(serializer.stats().ProduceFailedNoReferenceTime, 1);
}

TEST_F(HistogramSerializerTest, TestIntegerBinning) {
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};
  std::vector<Validator_t::ResultData_t> ExpectedResultData = {1, 1};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {2};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};

  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestNegativeIntegerBinning) {
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  std::map<int, int> testData = {{-33, 1}, {8, 1}, {-12, 1}, {12, 1}};
  std::vector<Validator_t::ResultData_t> ExpectedResultData = {1, 1};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {2};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};

  auto serializer = Validator.createHistogramSerializer();

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  for (auto &e : testData) {
    serializer.addEvent(e.first, e.second);
  }

  serializer.produce();
}

TEST_F(HistogramSerializerTest, TestHigherTimeThenPeriodDropped) {
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  std::map<int, int> testData = {{3, 1}, {8, 0}, {12, 1}, {25, 1}, {26, 1}};
  std::vector<Validator_t::ResultData_t> ExpectedResultData = {1, 3};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {4};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};

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
  using Validator_t = TestValidator<int64_t, float, uint64_t>;
  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  float step =
      static_cast<float>(CommonFbMembers.Period) / CommonFbMembers.BinSize;

  float minFloatStep =
      std::nextafter(step, std::numeric_limits<float>::max()) - step;
  std::map<float, int> testData = {
      {1.0, 1}, {step - minFloatStep, 1}, {step, 1}, {step + 1, 1}};

  std::vector<Validator_t::ResultData_t> ExpectedResultData = {2, 2};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {4};

  /// Initialize validator and create serializer
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};

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
  using Validator_t = TestValidator<int64_t, float, uint64_t>;

  std::vector<Validator_t::ResultData_t> ExpectedResultData = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = 
    {static_cast<Validator_t::ReferenceTime_t>(ESSTime::SecInNs.count() * 10)};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {4};

  /// Initialize validator and create serializer
  TestValidator Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};

  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  // Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  EXPECT_NO_THROW(serializer.produce());

  EXPECT_NO_THROW(serializer.produce());

  EXPECT_NO_THROW(serializer.produce());
}

TEST_F(HistogramSerializerTest, TestBufferAggregatedPulsesCalls) {

  //Setup callback for serializer.
  int invokeCount{0};
  auto HasBeenInvoked =  [&](bool) { ++invokeCount; };

  //We want pulse time to increment with 1 microseconds
  constexpr int LoopCount = 10;
  constexpr int BinInterval = 1000;
  constexpr int BinCount = 10;
  constexpr int AggregatedFrameCount = 10;
  esstime::TimeDurationNano IBMPulseInterval = 
    esstime::TimeDurationNano(BinInterval * BinCount);
  ESSTime current = ESSTime(0, 0);

  CommonFbMembers.Period = BinInterval;
  CommonFbMembers.AggregatedFrames = AggregatedFrameCount;
  /// Initialize validator and create serializer
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  Validator_t Validator{CommonFbMembers, HasBeenInvoked};

  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  //Setup serializer. First pulse after system start will never
  //be serialized. When a new pulse is received, the previous is serialized
  //and sent. Make a start pulse that will make system ready.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 0);

  // Perform test 1
  //Set data in the bin. Test data is 3 readout times with a values 300, 800, 1200. 
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin. 
  //If readout time is larger than the highest bin, it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index, 4, 9, 10. The bin index is given by index = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  //Pulse will be repeated 10 times and should only give a readout on 11th.
  std::map<int, int> testData = {{300, 10}, {800, 9}, {1200, 8}};
  //Since pulse values are summed the expected will be 10 * input
  std::vector<Validator_t::ResultData_t> validatorData = {0, 0, 0, 100, 0, 0, 0, 0, 90, 80};
  //Calculate pulse integration. Pulse data to validator is reference time and intensity values
  //Because of how ESS time operate with adding nano seconds following numbers will look strange
  std::vector<Validator_t::ReferenceTime_t> referenceData = {0, 9994, 19988, 29982, 39976, 49970, 59964, 69958, 79952, 89946};
  //Intensity is the sum of values in testData map 
  std::vector<Validator_t::SumValue_t> intensityData{27, 27, 27, 27, 27, 27, 27, 27, 27, 27};

  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);

  for (size_t i = 0; i < LoopCount; i++)
  {
    CommonFbMembers.setReferenceTime(current.toNS());
    current += IBMPulseInterval;
    for (const auto &[time, value] : testData) {
        serializer.addEvent(time, value);
    }
    serializer.checkAndSetReferenceTime(current.toNS());
    //Last check will result in a serialized payload
    EXPECT_EQ(invokeCount, i != 9 ? 0 : 1);
  }

  // Perform test 4
  //Set data in the bin again. Test data is 1 readout times with a values 700. 
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin. 
  //If readout time is larger than the highest bin, it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with the last bin strategy
  //is bin index 8. The bin index is given by bin = = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  //Since pulse values are summed the expected will be 10 * input
  Validator.setData(validatorData);
  serializer.addEvent(700, 10);
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 1);
  //Set it back to default for other unit test to have expected value
  CommonFbMembers.AggregatedFrames = 1;
}

TEST_F(HistogramSerializerTest, TestBufferAveragedPulsesCalls) {

  //Setup callback for serializer.
  int invokeCount{0};
  auto HasBeenInvoked =  [&](bool) { ++invokeCount; };

  //We want pulse time to increment with 1 microsecond
  constexpr int BinInterval = 1000;
  constexpr int BinCount = 10;
  constexpr int AggregatedFrameCount = 10;
  esstime::TimeDurationNano IBMPulseInterval = 
    esstime::TimeDurationNano(BinInterval * BinCount);
  ESSTime current = ESSTime(0, 0);

  CommonFbMembers.Period = BinInterval;
  CommonFbMembers.AggregatedFrames = AggregatedFrameCount;
  /// Initialize validator and create serializer
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  Validator_t Validator{CommonFbMembers, HasBeenInvoked};

  auto serializer = Validator.createHistogramSerializer(
    fbserializer::BinningStrategy::LastBin,
    essmath::AVERAGE_AGG_FUNC<int64_t>);

  //Setup serializer. First pulse after system start will never
  //be serialized. When a new pulse is received, the previous is serialized
  //and sent. Make a start pulse that will make system ready.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 0);

  // Perform test 1
  //Set data in the bin. Test data is 3 readout times with the values 300, 800, and 1200. 
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin. 
  //If readout time is larger than the highest bin, it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index, 4, 9, and 10. The bin index is given by index = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  //Pulse will be repeated 10 times and should only give a readout on 11th.
  std::map<int, int> testData = {{300, 10}, {800, 9}, {1200, 8}};
  //Runing average values will not be changed in bin
  std::vector<Validator_t::ResultData_t> validatorData = {0, 0, 0, 10, 0, 0, 0, 0, 9, 8};
  //Calculate pulse integration. Pulse data to validator is reference time and intensity values
  //Because of how ESS time operate with adding nano seconds following numbers will look strange
  std::vector<Validator_t::ReferenceTime_t> referenceData = {0, 9994, 19988, 29982, 39976, 49970, 59964, 69958, 79952, 89946};
  //Sum of validator data
  std::vector<Validator_t::SumValue_t> intensityData{27, 27, 27, 27, 27, 27, 27, 27, 27, 27};

  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);

  for (size_t i = 0; i < 10; i++)
  {
    CommonFbMembers.setReferenceTime(current.toNS());
    current += IBMPulseInterval;
    for (const auto &[time, value] : testData) {
        serializer.addEvent(time, value);
    }
    serializer.checkAndSetReferenceTime(current.toNS());
    //Last check will result in a serialized payload
    EXPECT_EQ(invokeCount, i != 9 ? 0 : 1);
  }

  // Perform test 4
  //Set data in the bin again. Test data is 1 readout times with a values 700.
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin.
  //If readout time is larger than the highest bin, it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with the last bin strategy
  //is bin index 8. The bin index is given by bin = = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  //Since pulse values are summed the expected will be 10 * input
  Validator.setData(validatorData);
  serializer.addEvent(700, 10);
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 1);
  //Set it back to default for other unit test to have expected value
  CommonFbMembers.AggregatedFrames = 1;
}

TEST_F(HistogramSerializerTest, TestBufferAggregatedPulsesCallsWithEmpty) {
  //Almost the same test as above but this one will have to pulses whit out any
  //Histogram data
  //Setup callback for serializer.
  int invokeCount{0};
  auto HasBeenInvoked =  [&](bool) { ++invokeCount; };

  //We want pulse time to increment with 1 microseconds
  constexpr int BinInterval = 1000;
  constexpr int BinCount = 10;
  constexpr int AggregatedFrameCount = 10;
  esstime::TimeDurationNano IBMPulseInterval = 
    esstime::TimeDurationNano(BinInterval * BinCount);
  ESSTime current = ESSTime(0, 0);

  CommonFbMembers.Period = BinInterval;
  CommonFbMembers.AggregatedFrames = AggregatedFrameCount;
  /// Initialize validator and create serializer
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  Validator_t Validator{CommonFbMembers, HasBeenInvoked};

  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  //Setup serializer. First pulse after system start will never
  //be serialized. When a new pulse is received, the previous pulse is serialized
  //and sent. Make a start pulse that will make system ready.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 0);

  // Perform test 1
  //Set data in the bin. Test data is 3 readout times with the values 300, 800, and 1200. 
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin. 
  //If the readout time is larger than the highest bin, it is added to the last bin BinningStrategy::LastBin.
  //Each readout time in test data, it is is divided with 100 bin. Expected data with the last bin strategy
  //is bin index, 4, 9, and 10. The bin index is index = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  //The pulse will be repeated 8 times and 2 times with empty histogram and only gives a readout on the 11th repeat.
  std::map<int, int> testData = {{300, 10}, {800, 9}, {1200, 8}};
  //Since pulse values are summed the expected will be 8 * input because of 2 empty
  std::vector<Validator_t::ResultData_t> validatorData = {0, 0, 0, 80, 0, 0, 0, 0, 72, 64};
  //Calculate pulse integration. Pulse data to validator is reference time and intensity values
  //Because of how ESS time operate with adding nano seconds following numbers will look strange
  std::vector<Validator_t::ReferenceTime_t> referenceData = {0, 9994, 19988, 29982, 39976, 49970, 59964, 69958, 79952, 89946};
  //At pulse 3 and 7 nothing is read. Other pulses will be sum of 10 + 9 + 8
  std::vector<Validator_t::SumValue_t> intensityData{27, 27, 27, 0, 27, 27, 27, 0, 27, 27};

  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);

  for (size_t i = 0; i < 10; i++)
  {
    CommonFbMembers.setReferenceTime(current.toNS());
    current += IBMPulseInterval;
    //Make two empty pulses
    if ((i != 3) && (i != 7)) {
      for (const auto &[time, value] : testData) {
        serializer.addEvent(time, value);
      }
    }
    serializer.checkAndSetReferenceTime(current.toNS());
    //Last check will result in a serialized payload
    EXPECT_EQ(invokeCount, i != 9 ? 0 : 1);
  }

  // Perform test 4
  //Set data in bin again. Test data is 1 readout times with a values 700. 
  //Expected result with current setup (where each bin is 100ns) must be in expect bin. 
  //If readout time is larger than highest bin it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index, 8. Bin index = readout time / 100 + 1. One is added because first bin
  //is pulse time + 0 readout.
  //This pulse should not trigger an message before 9 more are added.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  //Since pulse values are summed the expected will be 8 * input because of 2 empty
  Validator.setData({0, 0, 0, 100, 0, 0, 0, 0, 90, 80});
  serializer.addEvent(700, 10);
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 1);
  //Set it back to default for other unit test to have expected value
  CommonFbMembers.AggregatedFrames = 1;
}

TEST_F(HistogramSerializerTest, TestBufferAveragePulsesCallsWithEmpty) {
  //Almost the same test as above but this one will have to pulses whit out any
  //Histogram data
  //Setup callback for serializer.
  int invokeCount{0};
  auto HasBeenInvoked = [&](bool) { ++invokeCount; };

  //We want pulse time to increment with 1 microseconds
  constexpr int BinInterval = 1000;
  constexpr int BinCount = 10;
  constexpr int AggregatedFrameCount = 10;
  esstime::TimeDurationNano IBMPulseInterval = 
    esstime::TimeDurationNano(BinInterval * BinCount);
  ESSTime current = ESSTime(0, 0);

  CommonFbMembers.Period = BinInterval;
  CommonFbMembers.AggregatedFrames = AggregatedFrameCount;
  /// Initialize validator and create serializer
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  Validator_t Validator{CommonFbMembers, HasBeenInvoked};

  auto serializer = Validator.createHistogramSerializer(
    fbserializer::BinningStrategy::LastBin,
    essmath::AVERAGE_AGG_FUNC<int64_t>);

  //Setup serializer. First pulse after system start will never
  //be serialized. When a new pulse is received, the previous pulse is serialized
  //and sent. Make a start pulse that will make system ready.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 0);

  // Perform test 1
  //Set data in the bin. Test data is 3 readout times with the values 300, 800, and 1200. 
  //Expected result with current setup (where each bin is 100 ns) must be in the expected bin. 
  //If the readout time is larger than the highest bin, it is added to the last bin BinningStrategy::LastBin.
  //Each readout time in test data, it is is divided with 100 bin. Expected data with the last bin strategy
  //are the bin indices 4, 9, and 10. The bin index is index = readout time / 100 + 1. One is added because the first bin
  //is pulse time + 0 readout.
  //The pulse will be repeated 8 times and 2 times with empty histogram and only gives a readout on the 11'th repeat.
  std::map<int, int> testData = {{300, 10}, {800, 9}, {1200, 8}};
  //Since we use average the expected values are 
  std::vector<Validator_t::ResultData_t> validatorData = {0, 0, 0, 10, 0, 0, 0, 0, 9, 8};
  //Calculate pulse integration. Pulse data to validator is reference time and intensity values
  //Because of how ESS time operate with adding nano seconds following numbers will look strange
  std::vector<Validator_t::ReferenceTime_t> referenceData = {0, 9994, 19988, 29982, 39976, 49970, 59964, 69958, 79952, 89946};
  //At pulse 3 and 7 nothing is read. Other pulses will be sum of 10 + 9 + 8
  std::vector<Validator_t::SumValue_t> intensityData{27, 27, 27, 0, 27, 27, 27, 0, 27, 27};

  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);
  
  for (size_t i = 0; i < 10; i++)
  {
    CommonFbMembers.setReferenceTime(current.toNS());
    current += IBMPulseInterval;
    //Make two empty pulses
    if ((i != 3) && (i != 7)) {
      for (const auto &[time, value] : testData) {
        serializer.addEvent(time, value);
      }
    }
    serializer.checkAndSetReferenceTime(current.toNS());
    //Last check will result in a serialized payload
    EXPECT_EQ(invokeCount, i != 9 ? 0 : 1);
  }

  // Perform test 4
  //Set data in bin again. Test data is 1 readout times with a values 700. 
  //Expected result with current setup (where each bin is 100 ns) must be in expected bin. 
  //If readout time is larger than highest bin, it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index 8, where the bin index is given by index = readout time / 100 + 1. One is added 
  //because first bin is pulse time + 0 readout.
  //This pulse should not trigger a message before 9 more are added.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  //Since we use average the expected values are 
  Validator.setData(validatorData);
  serializer.addEvent(700, 10);
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_EQ(invokeCount, 1);
  //Set it back to default for other unit test to have expected value
  CommonFbMembers.AggregatedFrames = 1;
}

TEST_F(HistogramSerializerTest, TestBufferBinContentBehaviourCalls) {

  //Setup callback for serializer.
  bool invoked = false;
  auto HasBeenInvoked =  [&](bool isInvoked) { invoked = isInvoked; };

  //We want pulse time to increment with 1 microseconds
  constexpr int BinInterval = 1000;
  constexpr int BinCount = 10;
  esstime::TimeDurationNano IBMPulseInterval = 
    esstime::TimeDurationNano(BinInterval * BinCount);
  ESSTime current = ESSTime(0, 0);

  CommonFbMembers.Period = BinInterval;
  /// Initialize validator and create serializer
  using Validator_t = TestValidator<int64_t, double, uint64_t>;
  Validator_t Validator{CommonFbMembers, HasBeenInvoked};

  auto serializer = Validator.createHistogramSerializer(
      fbserializer::BinningStrategy::LastBin);

  // Perform test 1
  // Empty bins. Nothing serialized and callback method must not be invoked.
  // Test that callback flag is not set.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_FALSE(invoked);
  invoked = false;

  // Perform test 2
  //Set data in bin. Test data is 3 readout times with a values 300, 800, 1200. 
  //Expected result with current setup (where each bin is 100ns) must be  in expect bin. 
  //If readout time is larger than highest bin it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index, 4, 9, 10. Bin index = readout time / 100 + 1. One is added because first bin
  //is pulse time + 0 readout.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  std::map<int, int> testData = {{300, 10}, {800, 9}, {1200, 8}};
  
  std::vector<Validator_t::ResultData_t> validatorData = {0, 0, 0, 10, 0, 0, 0, 0, 9, 8};
  //Calculate pulse integration. Pulse data to validator is reference time and intensity values
  //Because of how ESS time operate with adding nano seconds following numbers will look strange
  std::vector<Validator_t::ReferenceTime_t> referenceData = {0};
  //At pulse 3 and 7 nothing is read. Other pulses will be sum of 10 + 9 + 8
  std::vector<Validator_t::SumValue_t> intensityData{27};

  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);

  for (auto &item : testData) {
    serializer.addEvent(item.first, item.second);
  }
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_TRUE(invoked);
  invoked = false;

  // Perform test 3
  // Empty bins again after a pulse with data to verify that internal empty bin flags are cleared after
  // data has been sent. Nothing serialized and callback method must not be invoked.
  // Test that callback flag is not set.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_FALSE(invoked);
  invoked = false;

  // Perform test 4
  //Set data in bin again. Test data is 1 readout times with a values 700. 
  //Expected result with current setup (where each bin is 100ns) must be in expect bin. 
  //If readout time is larger than highest bin it is added to last bin BinningStrategy::LastBin.
  //Each readout time in test data is divided with 100 bin. Expected data with last bin strategy
  //is bin index, 8. Bin index = readout time / 100 + 1. One is added because first bin
  //is pulse time + 0 readout.
  CommonFbMembers.setReferenceTime(current.toNS());
  current += IBMPulseInterval;
  validatorData = {0, 0, 0, 0, 0, 0, 0, 10, 0, 0};
  referenceData = {19988}; //29982
  intensityData = {10};
  Validator.setData(validatorData);
  Validator.setReference(referenceData);
  Validator.setIntensity(intensityData);
  serializer.addEvent(700, 10);
  serializer.checkAndSetReferenceTime(current.toNS());
  EXPECT_TRUE(invoked);
}

TEST_F(HistogramSerializerTest, TestReferenceTimeTriggersProduce) {
  using Validator_t = TestValidator<int64_t, double, uint64_t>;

  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};
  std::vector<Validator_t::ResultData_t> ExpectedResultData = {1, 1};
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {2};

  /// Setup test Condition
  CommonFbMembers.setPeriod(20).setBinSize(2).setReferenceTime(
      std::chrono::seconds(10));

  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
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
  using Validator_t = TestValidator<int64_t, int64_t, uint64_t>;
  /// Setup test condition
  CommonFbMembers.setPeriod(100)
      .setBinSize(10)
      .setReferenceTime(std::chrono::seconds(10))
      .setBinOffset(50); // Set BinOffset

  std::vector<Validator_t::ResultData_t> ExpectedResultData(CommonFbMembers.BinSize, 0);
  std::vector<Validator_t::ReferenceTime_t> ExpectedReferenceData = {0};
  std::vector<Validator_t::SumValue_t> ExpectedIntensityData = {2};

  ExpectedResultData[1] = 2; // Events at 55 and 59

  // Initialize validator and create serializer using
  // createHistogramSerializer()
  Validator_t Validator{CommonFbMembers, 
    ExpectedResultData,
    ExpectedReferenceData,
    ExpectedIntensityData};
    
  auto serializer = Validator.createHistogramSerializer();

  /// Perform test
  serializer.checkAndSetReferenceTime(CommonFbMembers.ReferenceTime);

  // Add events before and after the BinOffset
  serializer.addEvent(25, 1);  // Should be dropped due to BinOffset
  serializer.addEvent(55, 1);  // Should go into second bin
  serializer.addEvent(59, 1);  // Should go into second bin
  serializer.addEvent(150, 1); // Should be dropped due to period

  serializer.produce();

  EXPECT_EQ(serializer.stats().DataOverPeriodDropped,
            1); // One event before offset
  EXPECT_EQ(serializer.stats().DataOverPeriodLastBin, 0);
}

TEST_F(HistogramSerializerTest, TestAllowedTypeTemplates) {
  using InCompleteList_t = fbserializer::AllowedList<int8_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;
  using CompleteList_t = fbserializer::AllowedList<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double>;

  bool test = fbserializer::Is_Type_Allowed<int16_t, InCompleteList_t>::Valid;
  EXPECT_FALSE(test);
  test = fbserializer::Is_Type_Allowed<int16_t, CompleteList_t>::Valid;
  EXPECT_TRUE(test);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}