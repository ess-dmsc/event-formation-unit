// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <cstdint>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/LinearGenerator.h>
#include <gtest/gtest.h>

class LinearGeneratorTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

void ReadoutTest(uint8_t pulseCount, uint16_t frequency, double gradient,
                 uint32_t binCount) {
  double max = binCount * gradient;
  double binWidth = 1000.0 / frequency / binCount;

  LinearGenerator generator(frequency, binCount, gradient);

  size_t readoutCounter = 0;
  for (size_t i = 0; i < pulseCount * binCount; i++) {
    double xValue = generator.getValue();
    // xValue must be less than max pulse time. To avoid rounding errors
    // an epsilon value is added for rounding
    ASSERT_LE(xValue, max);

    // if counter is larger than readoutCounter then we start on a new pulse.
    if (readoutCounter >= binCount)
      readoutCounter = 0;
    double comparand = readoutCounter * gradient;
    ASSERT_EQ(xValue, comparand);

    double position = gradient * readoutCounter;
    if (static_cast<uint32_t>(position / binWidth) > binCount) {
      comparand = 0.0;
    } else {
      comparand = static_cast<uint32_t>(position / binWidth) * gradient;
    }

    double posValue = generator.getValueByPos(position);
    ASSERT_EQ(posValue, comparand);

    readoutCounter++;
  }
}

TEST_F(LinearGeneratorTest, TestConstructors) {
  // test max X value defined with unique gradient
  LinearGenerator generator(400000.0, 1000, 1000.0);  
  EXPECT_EQ(generator.getValueByPos(0), 0.000);
  EXPECT_EQ(generator.getValueByPos(800), 2000.000);
  EXPECT_EQ(generator.getValueByPos(399999), 999 * 1000.0);
  EXPECT_EQ(generator.getValueByPos(400000), 0.0);

  // test frequency defined instead max values and with unique gradient
  generator = LinearGenerator(static_cast<uint16_t>(25), 1000, 1000.0); 
  EXPECT_EQ(generator.getValueByPos(0), 0.000);
  EXPECT_EQ(generator.getValueByPos(0.8), 20 * 1000.000); 
  EXPECT_EQ(generator.getValueByPos(39.9999), 999 * 1000.0); 
  EXPECT_EQ(generator.getValueByPos(40.0), 0.0);

  /// Test frequency defined but gradient calculated
  generator = LinearGenerator(static_cast<uint16_t>(25), 1000);
  EXPECT_EQ(generator.getValueByPos(0), 0.000);
  EXPECT_EQ(generator.getValueByPos(0.8), 20 *  0.04);
  EXPECT_EQ(generator.getValueByPos(39.9999), 999 * 0.04);
  EXPECT_EQ(generator.getValueByPos(40.0), 0.0);
}

TEST_F(LinearGeneratorTest, TestReadoutsRolloverForMultiplePulses) {
  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 512);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 512);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 512);

  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 512);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 512);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 512);
}
