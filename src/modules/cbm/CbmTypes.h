// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM beam monitor type definitions and conversion functions
///
//===----------------------------------------------------------------------===//

#pragma once



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
/// CbmType cbmType("TTL");
/// int typeInt = static_cast<int>(cbmType);
/// std::string typeStr = cbmType.to_string();
///
/// CbmType cbmType2("GEM");
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
  /// \enum CbmTypes
  /// \brief Enumeration of CBM beam monitor types.
  enum CbmTypes {
    TTL = 0x01,   ///< TTL beam monitor type.
    N2GEM = 0x02, ///< N2GEM beam monitor type.
    IBM = 0x03,   ///< IBM beam monitor type.
    GEM = 0x04,   ///< GEM beam monitor type.
    FC = 0x05,    ///< FC beam monitor type.
    MM = 0x06     ///< MM beam monitor type.
  };

  /// \var MAX
  /// \brief Maximum CBM type value.
  static constexpr int MAX = CbmTypes::MM;

  /// \var MIN
  /// \brief Minimum CBM type value.
  static constexpr int MIN = CbmTypes::TTL;

  /// \brief Constructs a CbmType object from a string representation of the CBM type.
  /// \param typeStr The string representation of the CBM type.
  /// \throws std::invalid_argument if the provided typeStr is not a valid CBM type.
  CbmType(const std::string &typeStr) {
    if (typeStr == "TTL") {
      beamMonitorType = CbmTypes::TTL;
    } else if (typeStr == "N2GEM") {
      beamMonitorType = CbmTypes::N2GEM;
    } else if (typeStr == "IBM") {
      beamMonitorType = CbmTypes::IBM;
    } else if (typeStr == "GEM") {
      beamMonitorType = CbmTypes::GEM;
    } else if (typeStr == "FC") {
      beamMonitorType = CbmTypes::FC;
    } else if (typeStr == "MM") {
      beamMonitorType = CbmTypes::MM;
    } else {
      throw std::invalid_argument("Invalid CBM type string: " + typeStr);
    }
  }

  /// \brief Constructs a CbmType object from an integer representation of the CBM type.
  /// \param type_int The integer representation of the CBM type.
  /// \throws std::invalid_argument if the provided type_int is not a valid CBM type.
  CbmType(const int type_int) {
    if (type_int == 0x01) {
      beamMonitorType = CbmTypes::TTL;
    } else if (type_int == 0x02) {
      beamMonitorType = CbmTypes::N2GEM;
    } else if (type_int == 0x03) {
      beamMonitorType = CbmTypes::IBM;
    } else if (type_int == 0x04) {
      beamMonitorType = CbmTypes::GEM;
    } else if (type_int == 0x05) {
      beamMonitorType = CbmTypes::FC;
    } else if (type_int == 0x06) {
      beamMonitorType = CbmTypes::MM;
    } else {
      throw std::invalid_argument("Invalid CBM type integer: " +
                                  std::to_string(type_int));
    }
  }

  /// \brief Converts the CBM type to its string representation.
  /// \return The string representation of the CBM type.
  const char *to_string() const {
    switch (beamMonitorType) {
    case CbmTypes::TTL:
      return "TTL";
    case CbmTypes::N2GEM:
      return "N2GEM";
    case CbmTypes::IBM:
      return "IBM";
    case CbmTypes::GEM:
      return "GEM";
    case CbmTypes::FC:
      return "FC";
    case CbmTypes::MM:
      return "MM";
    default:
      return "Unknown";
    }
  }

private:
  CbmTypes beamMonitorType;

public:
  /// \brief Conversion operator to int.
  /// \return The integer representation of the CBM type.
  operator int() const { return static_cast<int>(beamMonitorType); }

  /// \brief Conversion operator to uint8_t.
  /// \return The uint8_t representation of the CBM type.
  operator uint8_t() const { return static_cast<uint8_t>(beamMonitorType); }

  /// \brief Overload of the equality operator (==) for comparing two CbmType objects.
  /// \param other The CbmType object to compare with.
  /// \return true if the CbmType objects are equal, false otherwise.
  bool operator==(const CbmType &other) const {
    return beamMonitorType == other.beamMonitorType;
  }

  /// \brief Overload of the equality operator (==) for comparing a CbmType object with a CbmTypes enum value.
  /// \param other The CbmTypes enum value to compare with.
  /// \return true if the CbmType object is equal to the CbmTypes enum value, false otherwise.
  bool operator==(const CbmTypes &other) const {
    return beamMonitorType == other;
  }
};

} // namespace cbm