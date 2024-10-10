// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator type definition and conversion functions
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <Error.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

/// \class GeneratorType
/// \brief Represents a generator type.
class GeneratorType {
public:
  /// \enum GeneratorTypes
  /// \brief Enumeration of generator types.
  enum GeneratorTypes {
    Distribution, ///< Distribution generator type.
    Linear,       ///< Linear generator type.
    Fixed         ///< Fixed generator type.
  };

  /// \var MAX
  /// \brief Maximum generator type value.
  static constexpr int MAX = GeneratorTypes::Fixed;

  /// \var MIN
  /// \brief Minimum generator type value.
  static constexpr int MIN = GeneratorTypes::Distribution;

  /// \brief Constructs a GeneratorType object from a GeneratorTypes enum value.
  explicit GeneratorType(GeneratorTypes type) : type_(type) {}

  /// \brief Constructs a GeneratorType object from a string representation of
  /// the generator type.
  explicit GeneratorType(const std::string &typeStr) {
    type_ = fromString(typeStr);
  }

  /// \brief Returns the string representation of the generator type.
  std::string toString() const {
    switch (type_) {
    case Distribution:
      return "Dist";
    case Linear:
      return "Linear";
    case Fixed:
      return "Fixed";
    default:
      throw CLI::ConversionError("Invalid generator type");
    }
  }

  /// \brief Returns the GeneratorTypes enum value from a string representation.
  static GeneratorTypes fromString(const std::string &typeStr) {
    if (typeStr == "Dist")
      return Distribution;
    if (typeStr == "Linear")
      return Linear;
    if (typeStr == "Fixed")
      return Fixed;
    throw CLI::ConversionError("Invalid generator type string");
  }

  /// \brief Returns the GeneratorTypes enum value.
  GeneratorTypes getType() const { return type_; }

  /// \brief Checks if the generator type is valid.
  static bool isValid(GeneratorTypes type) {
    return type >= MIN && type <= MAX;
  }

  /// \brief Equality operator overload for GeneratorType.
  /// \param other The other GeneratorType to compare with.
  /// \return True if the generator types are equal, false otherwise.
  bool operator==(const GeneratorType &other) const {
    return type_ == other.type_;
  }

  /// \brief Equality operator overload for GeneratorType.
  /// \param other The other type to compare with.
  /// \return True if the generator types are equal, false otherwise.
  bool operator==(const GeneratorType::GeneratorTypes &otherType) const {
    return type_ == otherType;
  }

private:
  GeneratorTypes type_;
};