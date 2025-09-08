// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Defines the GeometryType class representing different detector
/// geometry types for ESS readout systems
///
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace geometry {

/// \class GeometryType
///
/// \brief Type-safe wrapper for detector geometry types used in ESS readout systems.
///
/// This class provides a type-safe enumeration of detector geometry types with
/// conversion functions between different representations (string, integer, enum).
/// It supports case-insensitive string construction and validates input values
/// to prevent invalid geometry type assignments.
///
/// The class uses magic_enum for automatic string conversion and provides
/// comparison operators for easy usage in conditional statements and switch cases.
///
/// Features:
/// - Type-safe construction from strings (case-insensitive), integers, and enum values
/// - Automatic validation with exception throwing for invalid inputs
/// - String representation via toString() method
/// - Comparison operators for equality testing
/// - Static utility to get all available type names
///
/// \note This class replaces the previous UNKNOWN geometry type approach with
/// proper exception handling for invalid values.
///
/// Example usage:
/// \code
/// // Construction from different types
/// GeometryType GeomType("CAEN");           // From string (case-insensitive)
/// GeometryType GeomType2(1);               // From integer
/// GeometryType GeomType3(GeometryType::VMM3); // From enum
/// 
/// // String conversion
/// std::string typeStr = GeomType.toString(); // Returns "CAEN"
///
/// // Comparison
/// if (GeomType == GeometryType::CAEN) {
///     std::cout << "CAEN geometry detected" << std::endl;
/// }
///
/// // Error handling
/// try {
///     GeometryType invalid("INVALID");
/// } catch (const std::out_of_range& e) {
///     std::cout << "Invalid geometry type: " << e.what() << std::endl;
/// }
///
/// // Get all available types
/// std::string allTypes = GeometryType::getTypeNames(); // "CAEN, VMM3, DREAM, TREX, CBM"
/// \endcode

class GeometryType {

public:
  /// \enum Types
  /// \brief Enumeration of detector geometry types for different readout
  /// systems.
  enum Types {
    CAEN = 0,  ///< CAEN-based detectors (Loki, Bifrost, etc.)
    VMM3 = 1,  ///< VMM3-based detectors (NMX, Freia, etc.)
    DREAM = 2, ///< DREAM detector
    TREX = 3,  ///< T-REX detector
    CBM = 4    ///< CBM detector
  };

  /// \var MAX
  /// \brief Maximum geometry type value.
  static constexpr int MAX = Types::CBM;

  /// \var MIN
  /// \brief Minimum geometry type value.
  static constexpr int MIN = Types::CAEN;

  /// \brief Constructor from string name (case-insensitive).
  /// \param TypeName String representation of the geometry type (e.g., "CAEN", "caen", "VMM3")
  /// \throws std::out_of_range if TypeName is not a valid geometry type
  GeometryType(const std::string &TypeName) {
    // Convert to upper case, so both "caen" and "CAEN" will work
    std::string upper = TypeName;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    const auto t = magic_enum::enum_cast<Types>(upper);
    if (t.has_value()) {
      GeomType = t.value();
    } else {
      throw std::out_of_range("Invalid GeometryType string: " + TypeName);
    }
  }

  /// \brief Constructor from integer value.
  /// \param Type Integer value corresponding to a geometry type (0-4)
  /// \throws std::out_of_range if Type is not within the valid range [MIN, MAX]
  GeometryType(int Type = CAEN) {
    if (Type >= MIN && Type <= MAX) {
      GeomType = static_cast<Types>(Type);
    } else {
      throw std::out_of_range("Invalid GeometryType integer: " +
                              std::to_string(Type));
    }
  }

  /// \brief Constructor from Types enum value.
  /// \param Type Types enum value (defaults to CAEN)
  GeometryType(const Types Type = CAEN) : GeomType(Type) {}

  /// \brief Converts the geometry type to its string representation.
  /// \return The string representation of the geometry type.
  std::string toString() const {
    const std::string name(magic_enum::enum_name(GeomType));
    return name;
  }

private:
  Types GeomType;

public:
  /// \brief Conversion operator to Types enum for switch statements.
  /// \return The Types enum representation of the geometry type.
  operator Types() const { return GeomType; }

  /// \brief Overload of the equality operator (==) for comparing two
  /// GeometryType objects.
  /// \param other The GeometryType object to compare with
  /// \return true if the GeometryType objects are equal, false otherwise
  bool operator==(const GeometryType &other) const {
    return GeomType == other.GeomType;
  }

  /// \brief Overload of the equality operator (==) for comparing a GeometryType
  /// object with a Types enum value.
  /// \param Other The Types enum value to compare with
  /// \return true if the GeometryType object is equal to the Types enum value, false otherwise
  bool operator==(const Types &Other) const { return GeomType == Other; }

  /// \brief Gets a string representation of all available geometry types.
  /// \return A string containing all geometry type names separated by commas.
  static std::string getTypeNames() {
    std::string Result;
    auto Names = magic_enum::enum_names<Types>();
    for (size_t i = 0; i < Names.size(); ++i) {
      Result += std::string(Names[i]);
      if (i < Names.size() - 1) {
        Result += ", ";
      }
    }
    return Result;
  }
};

} // namespace geometry
