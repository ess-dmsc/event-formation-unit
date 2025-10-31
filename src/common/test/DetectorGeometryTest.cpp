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
class TestDetectorGeometry : public DetectorGeometry {
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
  uint32_t calcPixelImpl(const void *Data) const override {
    // Simple mock implementation that always returns a valid pixel
    if (Data == nullptr) {
      return 0;
    }
    const auto *caenData = static_cast<const DataParser::CaenReadout *>(Data);

    if (caenData->FiberId != 0) {
      return 0; // Simulate failure simulation for non-zero FiberId
    }

    return 1; // Simple valid pixel
  }

  /// \brief Mock implementation of validateDataType for testing
  /// \param type_info Type information from typeid()
  /// \return true only for CaenReadout, false for other types to test type
  /// validation
  bool validateDataType(const std::type_info &type_info) const override {
    // For CAEN geometry testing, only accept CAEN readout types
    // This allows us to test type validation failure with MockWrongReadout
    if (type_info == typeid(DataParser::CaenReadout)) {
      return true;
    }
    return false;
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
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 0);
}

/// \brief Test all validation functions using validateAll method
TEST_F(DetectorGeometryTest, ValidateAll) {
  // Test all valid values together
  bool allValid = caenGeometry->validateAll(
      [&]() { return caenGeometry->validateRing(10); },
      [&]() { return caenGeometry->validateFEN(5); });
  ASSERT_TRUE(allValid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 0);

  // Test with one invalid ring value (first validator fails - short-circuit)
  bool ringInvalid = caenGeometry->validateAll(
      [&]() {
        return caenGeometry->validateRing(-1);
      }, // Invalid - increments RingErrors
      [&]() {
        return caenGeometry->validateFEN(5);
      } // Not executed due to short-circuit
  );
  ASSERT_FALSE(ringInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors,
            0); // Still 0 due to short-circuit

  // Test with one invalid FEN value (first validator succeeds, second fails)
  bool fenInvalid = caenGeometry->validateAll(
      [&]() { return caenGeometry->validateRing(10); }, // Valid
      [&]() {
        return caenGeometry->validateFEN(12);
      } // Invalid - increments FENErrors
  );
  ASSERT_FALSE(fenInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 2);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors, 1);

  // Test with multiple invalid values - due to short-circuit, only first
  // executes
  bool multipleInvalid = caenGeometry->validateAll(
      [&]() {
        return caenGeometry->validateRing(24);
      }, // Invalid - increments RingErrors
      [&]() {
        return caenGeometry->validateFEN(-1);
      } // Not executed due to short-circuit
  );
  ASSERT_FALSE(multipleInvalid);
  ASSERT_EQ(caenGeometry->getBaseCounters().ValidationErrors, 3);
  ASSERT_EQ(caenGeometry->getBaseCounters().RingErrors, 2);
  ASSERT_EQ(caenGeometry->getBaseCounters().FENErrors,
            1); // Still 1, not incremented

  // Test individual validation boundary cases
  ASSERT_TRUE(caenGeometry->validateRing(0));   // Min valid ring
  ASSERT_TRUE(caenGeometry->validateRing(23));  // Max valid ring
  ASSERT_FALSE(caenGeometry->validateRing(-1)); // Below min (3rd ring error)
  ASSERT_FALSE(caenGeometry->validateRing(24)); // Above max (4th ring error)

  ASSERT_TRUE(caenGeometry->validateFEN(0));   // Min valid FEN
  ASSERT_TRUE(caenGeometry->validateFEN(11));  // Max valid FEN
  ASSERT_FALSE(caenGeometry->validateFEN(-1)); // Below min (2nd FEN error)
  ASSERT_FALSE(caenGeometry->validateFEN(12)); // Above max (3rd FEN error)

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
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 0);
}

/// \brief Test calcPixel method with CAEN readout that causes calculation
/// failure (PixelError)
TEST_F(DetectorGeometryTest, CalcPixelFailure) {
  DataParser::CaenReadout readout{10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  uint32_t pixel = caenGeometry->calcPixel(readout);
  ASSERT_EQ(pixel, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 0);
}

/// \brief Test calcPixel method with wrong readout type (TypeError)
/// This test verifies that the type validation works correctly by using
/// MockWrongReadout which should be rejected by validateDataType()
TEST_F(DetectorGeometryTest, CalcPixelWrongType) {
  MockWrongReadout wrongReadout{123, 456};

  uint32_t pixel = caenGeometry->calcPixel(wrongReadout);

  // MockWrongReadout should be rejected by validateDataType, causing TypeError
  ASSERT_EQ(pixel, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 1);
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors, 0);
}

/// \brief Test type validation system with multiple invalid types
TEST_F(DetectorGeometryTest, TypeValidationSystem) {
  // Test with first invalid type
  MockWrongReadout wrongReadout1{100, 200};
  uint32_t pixel1 = caenGeometry->calcPixel(wrongReadout1);
  ASSERT_EQ(pixel1, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 1);

  // Test with another completely different invalid type
  int invalidType = 42;
  uint32_t pixel2 = caenGeometry->calcPixel(invalidType);
  ASSERT_EQ(pixel2, 0);
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors, 2);

  // Test that valid CAEN type still works after invalid attempts
  DataParser::CaenReadout validReadout{0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 0, 0};
  uint32_t pixel3 = caenGeometry->calcPixel(validReadout);
  ASSERT_EQ(pixel3, 1); // Should succeed
  ASSERT_EQ(caenGeometry->getBaseCounters().TypeErrors,
            2); // No additional type error
  ASSERT_EQ(caenGeometry->getBaseCounters().PixelErrors,
            0); // No pixel calculation error
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
