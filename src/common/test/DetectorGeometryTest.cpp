// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for DetectorGeometry base class
///
//===----------------------------------------------------------------------===//

#include <common/geometry/DetectorGeometry.h>
#include <common/testutils/TestBase.h>
#include <modules/caen/readout/DataParser.h>

using namespace Caen;
using namespace geometry;

/// Mock readout data structure for testing wrong type
struct MockWrongReadout {
  uint32_t value1{0};
  uint32_t value2{0};
};

/// TestDetectorGeometry is a mock detector geometry class for testing purposes
/// It inherits from DetectorGeometry and can be used to create instances for
/// testing
class TestDetectorGeometry : public DetectorGeometry<DataParser::CaenReadout> {
public:
  /// \brief Constructor for the TestDetectorGeometry class
  explicit TestDetectorGeometry(Statistics &Stats)
      : DetectorGeometry(Stats, 23, 11) {
  } // Use standard CAEN limits: 24 rings (0-23), 12 FENs (0-11)

  /// \brief Destructor for the TestDetectorGeometry class
  virtual ~TestDetectorGeometry() = default;

  /// \brief Simple mock implementation of calcPixelImpl for testing
  /// \param Data Pointer to mock data (unused in this test)
  /// \return Fixed test value for validation
  uint32_t calcPixelImpl(const DataParser::CaenReadout &Data) const override {
    // Simple mock implementation that always returns a valid pixel
    if (Data.FiberId != 0) {
      return 0; // Simulate failure simulation for non-zero FiberId
    }

    return 1; // Simple valid pixel
  }

  // Public test wrapper methods that call protected validation methods
  // This keeps the base class methods protected while allowing testing

  /// \brief Public wrapper to test Ring validation
  bool testValidateRing(int Ring) const {
    return validateRing(Ring);
  }

  /// \brief Public wrapper to test FEN validation
  bool testValidateFEN(int FEN) const {
    return validateFEN(FEN);
  }

  /// \brief Public wrapper to test validateAll with Ring and FEN
  bool testValidateAll(int Ring, int FEN) const {
    return validateAll(
        [&]() { return validateRing(Ring); },
        [&]() { return validateFEN(FEN); });
  }

  /// \brief Public wrapper to test topology validation
  template <typename T>
  bool testValidateTopology(HashMap2D<T> &map, int Col, int Row) const {
    return validateTopology(map, Col, Row);
  }
};

class DetectorGeometryTest : public TestBase {
protected:
  Statistics Stats;
  std::unique_ptr<TestDetectorGeometry> caenGeometry;

  void SetUp() override {
    caenGeometry =
        std::make_unique<TestDetectorGeometry>(Stats);
  }

  void TearDown() override {}
};

/// \brief Test the constructor and initial state
TEST_F(DetectorGeometryTest, Constructor) {
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().TopologyError, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors, 0);
}

/// \brief Test all validation functions using validateAll method
TEST_F(DetectorGeometryTest, ValidateAll) {
  // Test all valid values together
  bool allValid = caenGeometry->testValidateAll(10, 5);
  ASSERT_TRUE(allValid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 0);

  // Test with one invalid ring value (first validator fails - short-circuit)
  bool ringInvalid = caenGeometry->testValidateAll(-1, 5);
  ASSERT_FALSE(ringInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors,
            0); // Still 0 due to short-circuit

  // Test with one invalid FEN value (first validator succeeds, second fails)
  bool fenInvalid = caenGeometry->testValidateAll(10, 12);
  ASSERT_FALSE(fenInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 2);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 1);

  // Test with multiple invalid values - due to short-circuit, only first
  // executes
  bool multipleInvalid = caenGeometry->testValidateAll(24, -1);
  ASSERT_FALSE(multipleInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 3);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 2);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors,
            1); // Still 1, not incremented

  // Test individual validation boundary cases
  ASSERT_TRUE(caenGeometry->testValidateRing(0));   // Min valid ring
  ASSERT_TRUE(caenGeometry->testValidateRing(23));  // Max valid ring
  ASSERT_FALSE(caenGeometry->testValidateRing(-1)); // Below min (3rd ring error)
  ASSERT_FALSE(caenGeometry->testValidateRing(24)); // Above max (4th ring error)

  ASSERT_TRUE(caenGeometry->testValidateFEN(0));   // Min valid FEN
  ASSERT_TRUE(caenGeometry->testValidateFEN(11));  // Max valid FEN
  ASSERT_FALSE(caenGeometry->testValidateFEN(-1)); // Below min (2nd FEN error)
  ASSERT_FALSE(caenGeometry->testValidateFEN(12)); // Above max (3rd FEN error)

  // Verify final counts
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 4);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 3);
  // direct validate functions above not increment validationErrors, counter
  // stays at 3
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 3);
}

TEST_F(DetectorGeometryTest, CalcPixelValidCAEN) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 0, 0};

  uint32_t pixel = caenGeometry->calcPixel(readout);
  ASSERT_EQ(pixel, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors, 0);
}

/// \brief Test calcPixel method with CAEN readout that causes calculation
/// failure (PixelError)
TEST_F(DetectorGeometryTest, CalcPixelFailure) {
  DataParser::CaenReadout readout{10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint32_t pixel = caenGeometry->calcPixel(readout);
  ASSERT_EQ(pixel, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
