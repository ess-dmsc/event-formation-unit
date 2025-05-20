// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM beam monitor type definitions and conversion functions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace cbm {

/// \class CbmType
///
/// \brief Represents the CBM (Common Beam Monitor type).
///
/// This class provides an enumeration of CBM beam monitor types and conversion
/// functions to convert between different representations of the CBM type.
///
/// The CBM types are expressed as an enumeration of CbmTypes and the necessary
/// conversion algorithms are provided by this class.
///
/// Furthermore this class also provides a to_string() function to convert the
/// CBM type to a string representation.
///

/// Example usage:
/// \code
/// CbmType cbmType("EVENT_0D");
/// int typeInt = static_cast<int>(cbmType);
/// std::string typeStr = cbmType.to_string();
///
/// CbmType cbmType2("2D");
/// if (cbmType == cbmType2) {
///     std::cout << "Both CBM types are the same." << std::endl;
/// } else {
///     std::cout << "CBM types are different." << std::endl;
/// }
/// \endcode

/// \see CbmTypes
///
/// \file CbmTypes.h
/// \brief Defines the CbmType class and its associated enumeration CbmTypes.

/// \class CbmType
/// \brief Represents a CBM type.
class CbmType {

public:
  /// \enum Types
  /// \brief Enumeration of CBM beam monitor types.
  enum Types {
    EVENT_0D = 0x01, ///< Event 0D type.
    EVENT_2D = 0x02, ///< Event 2D type.
    IBM = 0x03,      ///< IBM histogram type.
  };

  /// \var MAX
  /// \brief Maximum CBM type value.
  static constexpr int MAX = Types::IBM;

  /// \var MIN
  /// \brief Minimum CBM type value.
  static constexpr int MIN = Types::EVENT_0D;

  // Construct from string
  CbmType(const std::string &typeName) {
    // Convert to upper case, so both "value" and "VALUE" will work
    std::string upper = typeName;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    const auto t = magic_enum::enum_cast<Types>(upper);
    if (t.has_value()) {
      mBeamMonitorType = t.value();
    } else {
      throw std::out_of_range("Invalid CbmType string: " + typeName);
    }
  }

  // Construct from integer
  CbmType(const int type = EVENT_0D) {
    if (type >= MIN && type <= MAX) {
      mBeamMonitorType = static_cast<Types>(type);
    }

    else {
      throw std::out_of_range("Invalid CbmType integer: " +
                              std::to_string(type));
    }
  }

  /// \brief Converts the CBM type to its string representation.
  /// \return The string representation of the CBM type.
  std::string toString() const {
    const std::string name(magic_enum::enum_name(mBeamMonitorType));

    return name;
  }

private:
  Types mBeamMonitorType;

public:
  /// \brief Conversion operator to int.
  /// \return The integer representation of the CBM type.
  operator int() const { return static_cast<int>(mBeamMonitorType); }

  /// \brief Conversion operator to uint8_t.
  /// \return The uint8_t representation of the CBM type.
  operator uint8_t() const { return static_cast<uint8_t>(mBeamMonitorType); }

  /// \brief Overload of the equality operator (==) for comparing two CbmType
  /// objects. \param other The CbmType object to compare with. \return true if
  /// the CbmType objects are equal, false otherwise.
  bool operator==(const CbmType &other) const {
    return mBeamMonitorType == other.mBeamMonitorType;
  }

  /// \brief Overload of the equality operator (==) for comparing a CbmType
  /// object with a CbmTypes enum value. \param other The CbmTypes enum value to
  /// compare with. \return true if the CbmType object is equal to the CbmTypes
  /// enum value, false otherwise.
  bool operator==(const Types &other) const {
    return mBeamMonitorType == other;
  }
};

} // namespace cbm