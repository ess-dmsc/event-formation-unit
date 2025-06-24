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
                 uint32_t readoutsPerPulse) {
  double max = LinearGenerator::TIME_UNIT_MS / frequency;

  LinearGenerator generator(frequency, readoutsPerPulse, gradient);

  size_t readoutCounter = 0;
  for (size_t i = 0; i < pulseCount * readoutsPerPulse; i++) {
    double xValue = generator.getValue();
    double posValue = generator.getValueByPos(gradient * readoutCounter);
    // xValue must be less than max pulse time. To avoid rounding errors
    // an epsilon value is added for rounding
    ASSERT_LE(xValue, max);

    // if counter is larger than readoutCounter then we start on a new pulse.
    if (readoutCounter >= readoutsPerPulse)
      readoutCounter = 0;
    double xComperand = readoutCounter * gradient;

    ASSERT_EQ(xValue, xComperand);
    ASSERT_EQ(posValue, xComperand);

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
  generator = LinearGenerator(static_cast<uint16_t>(2500), 1000, 1000.0);
  EXPECT_EQ(generator.getValueByPos(0), 0.000);
  EXPECT_EQ(generator.getValueByPos(800), 2000.000);
  EXPECT_EQ(generator.getValueByPos(399999), 999 * 1000.0);
  EXPECT_EQ(generator.getValueByPos(400000), 0.0);

  /// Test frequncy defined but gradient calculated
  generator = LinearGenerator(static_cast<uint16_t>(2500), 1000);
  EXPECT_EQ(generator.getValueByPos(0), 0.000);
  EXPECT_EQ(generator.getValueByPos(800), 800.000);
  EXPECT_EQ(generator.getValueByPos(399999), 999 * 400.0);
  EXPECT_EQ(generator.getValueByPos(400000), 0.0);
}

TEST_F(LinearGeneratorTest, TestReadoutsRolloverForMultiplePulses) {
  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 71429);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 35715);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 2001);

  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 71429);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 35715);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 2001);
}
