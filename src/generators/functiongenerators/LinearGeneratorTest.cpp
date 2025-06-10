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
  void ReadoutTest(uint16_t frequency, double gradient) {
    double max = LinearGenerator::TimeDurationUnit / frequency;
    LinearGenerator generator(max, gradient);

    // Internally the generator contains a number of bins that build up
    // the linear curve
    double binDelta = max / (LinearGenerator::DEFAULT_BIN_COUNT);
    ASSERT_GT(binDelta, 0);

    size_t readoutCount = static_cast<size_t>(std::ceil(max / gradient));
    // if result of max/gradient is without decimals readout count needs to be 
    // increment to see that the distribution wraps around and return zero after the
    // readout is higher than pulse time.
    if (std::abs(std::ceil(max / gradient) - max / gradient) < 1E-5)
        ++readoutCount;

    for (size_t i = 0; i < readoutCount; i++) {
        double xValue = generator.getValue();
        //int offset = static_cast<int>(std::ceil(i * gradient / binDelta));
        double comperand = std::ceil(i * gradient / binDelta) * binDelta;
        ASSERT_NEAR(xValue, comperand, 1e-5);
    }
  }
}

TEST_F(LinearGeneratorTest, Constructors) {
  LinearGenerator generator(50000.0, 2000.0);
  ASSERT_NEAR(generator.getDistValue(0), 0.000, 1e-4);
}

TEST_F(LinearGeneratorTest, CheckReadouts) {
  ReadoutTest(ReadoutGeneratorBase::DEFAULT_FREQUENCY, 2000.0);
  ReadoutTest(25, 20000.0);
}
