// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief SchemaType definitions and conversion functions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>

namespace cbm {

/// \class SchemaType
///
/// \brief Represents the schema type used for serialize data for file writer.
///
/// This class provides an enumeration of schema types and conversion
/// functions to convert between different representations of the Schema type.
///
/// The Schema types are expressed as an enumeration of SchemaTypes and the necessary
/// conversion algorithms are provided by this class.
///
/// Furthermore this class also provides a to_string() function to convert the
/// Schema type to a string representation.
///

/// Example usage:
/// \code
/// SchemaType schemaType("EV44");
/// int typeInt = static_cast<int>(schemaType);
/// std::string typeStr = schemaType.to_string();
///
/// SchemaType schemaType2("DA00");
/// if (schemaType == schemaType2) {
///     std::cout << "Both Schema types are the same." << std::endl;
/// } else {
///     std::cout << "Schema types are different." << std::endl;
/// }
/// \endcode

/// \see SchemaTypes
///
/// \file SchemaTypes.h
/// \brief Defines the SchemaType class and its associated enumeration SchemaTypes.

/// \class SchemaType
/// \brief Represents a Schema type.
class SchemaType {

public:
  /// \enum Types
  /// \brief Enumeration of schema types.
  enum Types {
    DA00 = 0x00, ///< Use DA00HistogramSerializer.
    EV44 = 0x01, ///< Use EV44Serializer
  };

  /// \var MAX
  /// \brief Maximum Schema type value.
  static constexpr int MAX = EV44;

  /// \var MIN
  /// \brief Minimum Schema type value.
  static constexpr int MIN = DA00;

  // Construct from string
  SchemaType(const std::string &typeName) {
    // Convert to upper case, so both "value" and "VALUE" will work
    std::string upper = typeName;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    const auto type = magic_enum::enum_cast<Types>(upper);
    if (type.has_value()) {
      SchemaSerializeType = type.value();
    } else {
      throw std::out_of_range("Invalid SchemaType string: " + typeName);
    }
  }

  // Construct from integer
  SchemaType(const int type = DA00) {
    if (type >= MIN && type <= MAX) {
      SchemaSerializeType = static_cast<Types>(type);
    }

    else {
      throw std::out_of_range("Invalid SchemaType integer: " +
                              std::to_string(type));
    }
  }

  /// \brief Converts the Schema type to its string representation.
  /// \return The string representation of the Schema type.
  std::string toString() const {
    const std::string name(magic_enum::enum_name(SchemaSerializeType));

    return name;
  }

private:
  Types SchemaSerializeType;

public:
  /// \brief Conversion operator to int.
  /// \return The integer representation of the Schema type.
  operator int() const { return static_cast<int>(SchemaSerializeType); }

  /// \brief Overload of the equality operator (==) for comparing two SchemaType
  /// objects. \param other The SchemaType object to compare with. \return true if
  /// the SchemaType objects are equal, false otherwise.
  bool operator==(const SchemaType &other) const {
    return *this == other.SchemaSerializeType;
  }

  /// \brief Overload of the equality operator (==) for comparing a SchemaType
  /// object with a CbmTypes enum value. \param other The CbmTypes enum value to
  /// compare with. \return true if the SchemaType object is equal to the CbmTypes
  /// enum value, false otherwise.
  bool operator==(const Types &other) const {
    return SchemaSerializeType == other;
  }

  /// \brief Gets a string representation of all available SchemaType types.
  /// \return A string containing all Schema type names separated by commas.
  static std::string getTypeNames() {
    std::string result;
    auto names = magic_enum::enum_names<Types>();
    for (size_t i = 0; i < names.size(); ++i) {
      result += std::string(names[i]);
      if (i < names.size() - 1) {
        result += ", ";
      }
    }
    return result;
  }
};

} // namespace cbm