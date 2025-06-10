// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/LinearGenerator.h>

class LinearGeneratorTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

namespace {
  void ReadoutTest(uint8_t pulseCount, uint16_t frequency, double gradient, uint32_t readoutsPerPulse, double offset = 0.0) {
    double max = LinearGenerator::LINEAR_TIME_UNIT / frequency;
    LinearGenerator generator(max, gradient, offset);

    // Internally the generator contains a number of bins that build up
    // the linear curve
    double binDelta = max / gradient;
    ASSERT_GT(binDelta, 0);

    size_t readoutCount = static_cast<size_t>(std::ceil(max / gradient));
    // if result of max/gradient is without decimals readout count needs to be 
    // increment to see that the distribution wraps around and return zero after the
    // readout is higher than pulse time.
    if (std::abs(std::ceil(max / gradient) - max / gradient) < 1E-5)
        ++readoutCount;
    
    size_t counter = 0;
    for (size_t i = 0; i < pulseCount * readoutCount; i++) {
        double xValue = generator.getValue();
        //xValue must be less than max pulse time. To avoid rounding errors 
        //an epsilon value is added for rounding
        ASSERT_LT(xValue, max + 1e-3);

        // if counter is larger than readoutCount then we start on a new pulse.
        if (counter >= readoutsPerPulse)
          counter = 0; 
        double xComperand = std::ceil(counter++ * gradient / binDelta) * binDelta;
        ASSERT_NEAR(xValue, xComperand, 1e-5);

        double yValue = generator.getValueByIndex(xValue);
        double yComperand = static_cast<int>(xValue / binDelta) * binDelta + offset;
        ASSERT_NEAR(yValue, yComperand, 1e-5);
    }
  }
}

TEST_F(LinearGeneratorTest, Constructors) {
  LinearGenerator generator(50000.0, 2000.0);
  ASSERT_NEAR(generator.getValueByIndex(0), 0.000, 1e-4);
}

TEST_F(LinearGeneratorTest, CheckReadouts) {
  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 71429);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 35715);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 2001);

  // Test with gradient of 1 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 1000.0, 71429, 456789.0);
  // Test with gradient of 2 microseconds
  ReadoutTest(4, ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0, 35715, 987654.0);
  // Test with gradient of 20 microseconds
  ReadoutTest(6, 25, 20000.0, 2001, 987456.0);

}
