// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file DetectorType.h
///
/// \brief Detector type enumeration and utilities for the Event Formation Unit (EFU)
///
/// This file defines the DetectorType class which provides type-safe detector 
/// identification with string conversion capabilities and comparison operators.
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

/// \brief Type-safe detector identification class with string conversion support
///
/// DetectorType provides a strongly-typed enumeration for detector types used
/// in the EFU system. It supports construction from strings or integers,
/// case-insensitive string comparisons, and automatic conversion to uint8_t
/// for protocol compatibility.
///
/// \For Example Usage:
/// \code
/// DetectorType detector("FREIA");        // From string (case-insensitive)
/// DetectorType detector2(0x48);          // From integer value
/// 
/// if (detector == "freia") {             // Case-insensitive comparison
///     std::cout << detector.toString();  // Outputs: "FREIA"
/// }
/// 
/// uint8_t value = detector;              // Implicit conversion to uint8_t
/// \endcode
class DetectorType {
public:

  /// \brief Enumeration of supported detector types
  ///
  /// Each detector type has a unique hexadecimal identifier used in
  /// communication protocols and data headers.
  enum Types {
    RESERVED = 0x00,
    CBM      = 0x10,
    LOKI     = 0x30,
    TBL3HE   = 0x32,
    BIFROST  = 0x34,
    MIRACLES = 0x38,
    CSPEC    = 0x3C,
    TREX     = 0x40,
    NMX      = 0x44,
    FREIA    = 0x48,
    TBLMB    = 0x49,
    ESTIA    = 0x4C,
    BEER     = 0x50,
    DREAM    = 0x60,
    MAGIC    = 0x64,
    HEIMDAL  = 0x68
  };

  /// \brief Minimum valid detector type value
  static constexpr int MIN = Types::RESERVED;
  /// \brief Maximum valid detector type value  
  static constexpr int MAX = Types::HEIMDAL;

  /// \brief Construct DetectorType from string name
  ///
  /// Creates a DetectorType from a string representation of the detector name.
  /// The comparison is case-insensitive, so "freia", "FREIA", and "Freia" 
  /// are all equivalent.
  ///
  /// \param typeName String name of the detector type (case-insensitive)
  /// \throws std::out_of_range if the string does not match any known detector type
  ///
  /// \For Example:
  /// \code
  /// DetectorType detector("freia");  // Case-insensitive
  /// DetectorType detector2("FREIA"); // Also works
  /// \endcode
  DetectorType(const std::string &typeName) {
    // Convert to upper case, so both "value" and "VALUE" will work
    std::string upper = typeName;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    const auto t = magic_enum::enum_cast<Types>(upper);
    if (t.has_value()) {
      mDetectorType = t.value();
    } else {
      throw std::out_of_range("Invalid DetectorType string: " + typeName);
    }
  }

  /// \brief Construct DetectorType from integer value
  ///
  /// Creates a DetectorType from its numeric identifier. The value must be
  /// within the valid range [MIN, MAX] as defined by the Types enumeration.
  ///
  /// \param type Integer identifier for the detector type (default: RESERVED)
  /// \throws std::out_of_range if the integer value is outside valid range
  ///
  /// \For Example:
  /// \code
  /// DetectorType detector(0x48);        // FREIA detector
  /// DetectorType defaultType();         // RESERVED detector
  /// \endcode
  DetectorType(int type=RESERVED) {
    if (type >= MIN && type <= MAX) {
      mDetectorType = static_cast<Types>(type);
    }

    else {
      throw std::out_of_range("Invalid DetectorType integer: " +
                                  std::to_string(type));
    }
  }

  /// \brief Convert DetectorType to string representation
  ///
  /// Returns the string name of the detector type in uppercase format.
  /// This is the canonical string representation used throughout the system.
  ///
  /// \return String name of the detector type (e.g., "FREIA", "ESTIA")
  ///
  /// \For Example:
  /// \code
  /// DetectorType detector("freia");
  /// std::string name = detector.toString(); // Returns "FREIA"
  /// \endcode
  std::string toString() const {
    const std::string name(magic_enum::enum_name(mDetectorType));

    return name;
  }

  /// \brief Convert DetectorType to uppercase string
  ///
  /// Returns the detector type name in uppercase format. This method is
  /// equivalent to toString() and exists for explicit uppercase conversion.
  ///
  /// \return Uppercase string name of the detector type
  std::string toUpperCase() const {
    return toString();
  }

  /// \brief Convert DetectorType to lowercase string
  ///
  /// Returns the detector type name in lowercase format for cases where
  /// lowercase representation is needed.
  ///
  /// \return Lowercase string name of the detector type (e.g., "freia", "estia")
  ///
  /// \par Example:
  /// \code
  /// DetectorType detector(DetectorType::FREIA);
  /// std::string lower = detector.toLowerCase(); // Returns "freia"
  /// \endcode
  std::string toLowerCase() const {
    // Convert to lower case
    std::string lower = toString();
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower;
  }

  /// \brief Implicit conversion to uint8_t
  ///
  /// Provides automatic conversion to the underlying 8-bit integer value
  /// for use in protocols and data structures that require numeric identifiers.
  ///
  /// \return 8-bit unsigned integer representation of the detector type
  operator uint8_t() const { return static_cast<uint8_t>(mDetectorType); }

  /// \brief Equality comparison with another DetectorType
  ///
  /// \param other DetectorType to compare against
  /// \return true if both DetectorType instances represent the same detector
  bool operator==(const DetectorType &other) const {
    return mDetectorType == other.mDetectorType;
  }

  /// \brief Case-insensitive equality comparison with string
  ///
  /// Compares this DetectorType with a string representation. The comparison
  /// is case-insensitive, allowing flexible string matching.
  ///
  /// \param other String to compare against (case-insensitive)
  /// \return true if the string matches this detector type
  ///
  /// \par Example:
  /// \code
  /// DetectorType detector(DetectorType::FREIA);
  /// bool match1 = (detector == "freia");  // true
  /// bool match2 = (detector == "FREIA");  // true
  /// bool match3 = (detector == "estia");  // false
  /// \endcode
  bool operator==(const std::string &other) const {
    std::string upper = other;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return toUpperCase() == upper;
  }

  /// \brief Case-insensitive inequality comparison with string
  ///
  /// Compares this DetectorType with a string representation for inequality.
  /// The comparison is case-insensitive.
  ///
  /// \param other String to compare against (case-insensitive)
  /// \return true if the string does not match this detector type
  bool operator!=(const std::string &other) const {
    return !(*this == other);
  }

  /// \brief Inequality comparison with another DetectorType
  ///
  /// \param other DetectorType to compare against
  /// \return true if the DetectorType instances represent different detectors
  bool operator!=(const DetectorType &other) const {
    return mDetectorType != other.mDetectorType;
  }

  /// \brief Equality comparison with Types enum value
  ///
  /// \param other Types enum value to compare against
  /// \return true if this DetectorType matches the enum value
  bool operator==(const Types &other) const {
    return mDetectorType == other;
  }

  /// \brief Inequality comparison with Types enum value
  ///
  /// \param other Types enum value to compare against
  /// \return true if this DetectorType does not match the enum value
  bool operator!=(const Types &other) const {
    return mDetectorType != other;
  }

private:
  Types mDetectorType;  ///< Internal storage for the detector type
};
