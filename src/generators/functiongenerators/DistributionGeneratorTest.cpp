// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

class DistributionGeneratorTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

///\brief not the best of test, just checking end of ranges with current
// known values
TEST_F(DistributionGeneratorTest, Constructors) {
  DistributionGenerator MyDist(1000.0/ReadoutGeneratorBase::DEFAULT_FREQUENCY);
  ASSERT_NEAR(MyDist.Dist[0], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.Dist[MyDist.DEFAULT_BIN_COUNT - 1], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.CDF[0], 0.000, 1e-4);
  ASSERT_NEAR(MyDist.CDF[1], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.CDF[MyDist.DEFAULT_BIN_COUNT - 1], 11.957, 0.005);
}

TEST_F(DistributionGeneratorTest, ConstructorWithMaxVal) {
  double MaxVal = 1000.0;
  int Bins = DistributionGenerator::DEFAULT_BIN_COUNT;
  DistributionGenerator MyDist(MaxVal);

  ASSERT_EQ(MyDist.MaxRange, MaxVal);
  ASSERT_EQ(MyDist.DEFAULT_BIN_COUNT, Bins);
  ASSERT_EQ(MyDist.Dist.size(), Bins);
  ASSERT_EQ(MyDist.CDF.size(), Bins);
  ASSERT_NEAR(MyDist.BinWidth, MaxVal / (Bins - 1), 1e-6);
  ASSERT_NEAR(MyDist.Norm, MyDist.CDF[Bins - 1], 1e-6);
}

TEST_F(DistributionGeneratorTest, ConstructorWithMaxValDefaultBins) {
  double MaxVal = 1000.0;
  int DefaultBins = DistributionGenerator::DEFAULT_BIN_COUNT;
  DistributionGenerator MyDist(MaxVal);

  ASSERT_EQ(MyDist.MaxRange, MaxVal);
  ASSERT_EQ(MyDist.DEFAULT_BIN_COUNT, DefaultBins);
  ASSERT_EQ(MyDist.Dist.size(), DefaultBins);
  ASSERT_EQ(MyDist.CDF.size(), DefaultBins);
  ASSERT_NEAR(MyDist.BinWidth, MaxVal / (DefaultBins - 1), 1e-6);
  ASSERT_NEAR(MyDist.Norm, MyDist.CDF[DefaultBins - 1], 1e-6);
}

// This test does not make sense. Negative bin 
TEST_F(DistributionGeneratorTest, ConstructorWithNegativeBins) {
  double MaxVal = 1000.0;
  int NegativeBins = -DistributionGenerator::DEFAULT_BIN_COUNT;
  int AbsBins = abs(NegativeBins);
  DistributionGenerator MyDist(MaxVal);

  ASSERT_EQ(MyDist.MaxRange, MaxVal);
  ASSERT_EQ(MyDist.DEFAULT_BIN_COUNT, AbsBins); // Bins should be set to absolute value of NegativeBins
  ASSERT_EQ(MyDist.Dist.size(), AbsBins);
  ASSERT_EQ(MyDist.CDF.size(), AbsBins);
  ASSERT_NEAR(MyDist.BinWidth, MaxVal / (AbsBins - 1), 1e-6);
  ASSERT_NEAR(MyDist.Norm, MyDist.CDF[AbsBins - 1], 1e-6);
}

TEST_F(DistributionGeneratorTest, GetValueWithinRange) {
  double MaxVal = 1000.0;
  DistributionGenerator MyDist(MaxVal);

  double value = MyDist.getValueByIndex(500.0) * MaxVal;
  ASSERT_GE(value, 0.0);
  ASSERT_LE(value, MaxVal);
}

TEST_F(DistributionGeneratorTest, GetValueHigherThanMaxVal) {
  double MaxVal = 1000.0;
  DistributionGenerator MyDist(MaxVal);

  double value = MyDist.getValueByIndex(MaxVal + 100.0) * MaxVal;
  ASSERT_GE(value, 0.0);
  ASSERT_LE(value, MaxVal);
}

TEST_F(DistributionGeneratorTest, GetValueRandom) {
  double MaxVal = 1000.0;
  DistributionGenerator MyDist(MaxVal);

  double value1 = MyDist.getValue();
  double value2 = MyDist.getValue();
  ASSERT_GE(value1, 0.0);
  ASSERT_LE(value1, MaxVal);
  ASSERT_GE(value2, 0.0);
  ASSERT_LE(value2, MaxVal);
  ASSERT_NE(value1, value2);
}

TEST_F(DistributionGeneratorTest, OneMillionRandomValues) {
  double MaxVal = 1000.0;
  DistributionGenerator Dist(MaxVal);

  for (size_t i = 0; i < 100000; ++i) {
    const double value = Dist.getValue();
    ASSERT_GE(value, 0.0);
    ASSERT_LE(value, MaxVal);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}