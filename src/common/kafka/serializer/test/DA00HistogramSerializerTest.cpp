// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of Frame1DHistogramSerializer class
///
//===----------------------------------------------------------------------===//

#include <cmath>
#include "common/kafka/serializer/DA00HistogramSerializer.h"
#include "common/math/NumericalMath.h"
#include "common/testutils/TestBase.h"

template <typename T>
class HistogramSerializerTester : public fbserializer::HistogramSerializer<T> {

public:
  HistogramSerializerTester(const std::string &Topic, const std::string &Source,
                            int32_t &Period, const int32_t &BinCount,
                            const std::string Name, const std::string Unit,
                            fbserializer::HistrogramSerializerStats &Stats,
                            const ProducerCallback Callback)
      : fbserializer::HistogramSerializer<T>(Topic, Source, Period, BinCount, Name, Unit,
                               "millisecond", Stats, Callback) {}

  std::vector<std::vector<T>> &getInternalData() { return this->DataBins; }
  std::vector<T> &getInternalXAxis() { return this->XAxisValues; }
};

class HistogramSerializerTest : public TestBase {

protected:
  fbserializer::HistrogramSerializerStats Stats;

  void SetUp() override { Stats = fbserializer::HistrogramSerializerStats(); }

  void TearDown() override {}
};

void function(nonstd::span<const uint8_t> data, int64_t timestamp) {
  std::cout << "Timestamp: " << timestamp << " Data size: " << data.size()
            << std::endl;
  uint size = 536;
  EXPECT_EQ(data.size(), size);
}

TEST_F(HistogramSerializerTest, TestIntConstructor) {
  int32_t period = 1000;
  int32_t count = 10;
  auto serializer =
      HistogramSerializerTester<uint64_t>("some topic", "source", period, count,
                                          "testData", "adc", Stats, function);

  EXPECT_EQ(serializer.getInternalData().size(), count);
  EXPECT_EQ(serializer.getInternalXAxis().size(), count);
  for (size_t i = 0; i < serializer.getInternalXAxis().size(); ++i) {
    EXPECT_EQ((serializer.getInternalXAxis())[i],
              i * static_cast<uint64_t>(period) / count);
  }
}

TEST_F(HistogramSerializerTest, TestIntConstructorNegativeValues) {
  int32_t period = -1000;
  int32_t count = 10;
  EXPECT_THROW(HistogramSerializerTester<uint64_t>("some topic", "source",
                                                   period, count, "testData",
                                                   "adc", Stats, function),
               std::domain_error);

  period = 1000;
  count = -10;
  EXPECT_THROW(HistogramSerializerTester<uint64_t>("some topic", "source",
                                                   period, count, "testData",
                                                   "adc", Stats, function),
               std::domain_error);
}

TEST_F(HistogramSerializerTest, TestUint64Uint64Constructor) {
  int32_t period = 1000000000;
  int32_t count = 400;
  auto serializer =
      HistogramSerializerTester<uint64_t>("some topic", "source", period, count,
                                          "testData", "adc", Stats, function);

  EXPECT_EQ(serializer.getInternalData().size(), count);
  EXPECT_EQ(serializer.getInternalXAxis().size(), count);
  for (size_t i = 0; i < serializer.getInternalXAxis().size(); ++i) {
    EXPECT_EQ((serializer.getInternalXAxis())[i],
              i * static_cast<uint64_t>(period) / count);
  }
}

TEST_F(HistogramSerializerTest, TestFloatConstructor) {
  int32_t period = 100;
  int32_t count = 30;
  auto serializer =
      HistogramSerializerTester<float>("some topic", "source", period, count,
                                       "testData", "adc", Stats, function);

  EXPECT_EQ(serializer.getInternalData().size(), count);
  EXPECT_EQ(serializer.getInternalXAxis().size(), count);

  float step = static_cast<float>(period) / static_cast<float>(count);

  float value = 0;
  for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
    EXPECT_EQ(serializer.getInternalXAxis()[i], value);
    value += step;
  }
}

TEST_F(HistogramSerializerTest, TestDoubleConstructor) {
  int32_t period = 1;
  int32_t count = 30;
  auto serializer =
      HistogramSerializerTester<double>("some topic", "source", period, count,
                                        "testData", "adc", Stats, function);

  EXPECT_EQ(serializer.getInternalData().size(), count);
  EXPECT_EQ(serializer.getInternalXAxis().size(), count);

  double step = static_cast<double>(period) / static_cast<double>(count);

  double value = 0;
  for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
    EXPECT_EQ(serializer.getInternalXAxis()[i], value);
    value += step;
  }
}

TEST_F(HistogramSerializerTest, EDGETestIntConstructorWithFractional) {
  int32_t period = 1;
  int32_t count = 30;
  auto serializer =
      HistogramSerializerTester<uint>("some topic", "source", period, count,
                                      "testData", "adc", Stats, function);

  EXPECT_EQ(serializer.getInternalData().size(), count);
  EXPECT_EQ(serializer.getInternalXAxis().size(), count);

  uint step = static_cast<uint>(period) / static_cast<uint>(count);

  uint value = 0;
  for (size_t i = 0; i < serializer.getInternalXAxis().size(); i++) {
    EXPECT_EQ(serializer.getInternalXAxis()[i], value);
    value += step;
  }
}

TEST_F(HistogramSerializerTest, TestIntegerBinning) {
  int32_t period = 20;
  int32_t count = 2;
  auto serializer =
      HistogramSerializerTester<int>("some topic", "source", period, count,
                                     "testData", "adc", Stats, function);

  // std::map<int, int> testData = {{10, 0}, {20, 1}, {30, 1}, {40, 0}, {50, 0},
  //                                {60, 1}, {70, 0}, {80, 0}, {90, 1}, {100,
  //                                0}};

  std::map<int, int> testData = {{3, 0}, {8, 1}, {12, 1}};

  for (auto &e : testData) {
    serializer.addData(e.first, e.second);
  }

  EXPECT_EQ(serializer.getInternalData().size(), count);
  for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
    EXPECT_EQ(essmath::SUM_AGG_FUNC<int>(serializer.getInternalData()[i]), 1);
  }
}

TEST_F(HistogramSerializerTest, EdgeTestFractionalBinning) {
  int32_t period = 20;
  int32_t count = 3;
  float step = static_cast<float>(period) / static_cast<float>(count);

  auto serializer =
      HistogramSerializerTester<float>("some topic", "source", period, count,
                                       "testData", "adc", Stats, function);

  float minFloatStep =
      std::nextafter(step, std::numeric_limits<float>::max()) - step;

  std::map<float, int> testData = {
      {1.0, 0}, {step - minFloatStep, 1}, {step, 1}, {step * 2, 1}};

  for (auto &e : testData) {
    serializer.addData(e.first, e.second);
  }

  EXPECT_EQ(serializer.getInternalData().size(), count);
  for (size_t i = 0; i < serializer.getInternalData().size(); i++) {
    std::cout << "Loop status: " << i + 1 << "/"
              << serializer.getInternalData().size() << std::endl;
    EXPECT_EQ(essmath::SUM_AGG_FUNC<float>(serializer.getInternalData()[i]), 1);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}