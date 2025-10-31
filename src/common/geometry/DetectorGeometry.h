// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common detector geometry base class with automatic counter
/// registration
///
/// This class provides a unified foundation for detector geometries that use
/// the ESS Ring/FEN structure. It eliminates code duplication across detector
/// implementations by providing common validation logic and automatic counter
/// registration.
///
/// Key benefits:
/// - Automatic counter registration using StatCounterBase
/// - Common Ring/FEN validation with error tracking
/// - Consistent counter naming across all detectors
/// - Simplified interface - no pure virtual functions
/// - Extensible configuration methods
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/StatCounterBase.h>
#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/memory/HashMap2D.h>
#include <string>
#include <typeinfo>

///
/// \brief Common detector geometry base class for ESS Ring/FEN detectors
///
/// This class provides shared functionality for detectors that use the ESS
/// Ring/FEN readout structure. It automatically registers consistent counters
/// and provides common validation methods to eliminate code duplication.
///
/// Usage:
/// 1. Inherit from DetectorGeometry
/// 2. Implement calcPixelImpl(void* Data) for your specific data type (cast
/// internally)
/// 3. Call calcPixel<T>(Data) to get automatic error counting
/// 4. Error counters are automatically updated and registered
///

namespace geometry {

class DetectorGeometry {
  // Public static keys for StatCounterBase map entries. Use inline to
  // allow header-only definition without ODR issues (C++17 and later).

  // clang-format off
public:
  static inline const std::string METRIC_RING_ERRORS{"geometry.ring_errors"};
  static inline const std::string METRIC_FEN_ERRORS{"geometry.fen_errors"};
  static inline const std::string METRIC_RING_MAPPING_ERRORS{"geometry.ring_mapping_errors"};
  static inline const std::string METRIC_TOPOLOGY_ERRORS{"geometry.topology_errors"};
  static inline const std::string METRIC_VALIDATION_ERRORS{"geometry.validation_errors"};
  static inline const std::string METRIC_PIXEL_ERRORS{"geometry.pixel_errors"};
  static inline const std::string METRIC_READOUT_TYPE_ERRORS{"geometry.readout_type_errors"};
  // clang-format on

protected:
  const int MaxRing{0};        ///< Maximum valid Ring value (exclusive)
  const int MaxFEN{0};         ///< Maximum valid FEN value (exclusive)

  ///
  /// \brief Geometry statistics counters with automatic registration
  ///
  /// These counters are automatically registered with consistent naming
  /// across all detector types using the StatCounterBase mechanism.
  ///
  struct BaseGeometryCounters : public StatCounterBase {
    int64_t RingErrors{0};        ///< Ring validation failures
    int64_t FENErrors{0};         ///< FEN validation failures
    int64_t RingMappingErrors{0}; ///< Ring configuration issues
    int64_t TopologyError{0};     ///< Topology configuration issues
    int64_t ValidationErrors{0};  ///< General validation failures
    int64_t PixelErrors{0};       ///< Pixel calculation failures
    int64_t TypeErrors{0};        ///< Type validation failures

    ///
    /// \brief Constructor with automatic counter registration
    /// \param Stats Reference to Statistics object
    ///
    BaseGeometryCounters(Statistics &Stats)
        : StatCounterBase(Stats,
                          {{METRIC_RING_ERRORS, RingErrors},
                           {METRIC_FEN_ERRORS, FENErrors},
                           {METRIC_RING_MAPPING_ERRORS, RingMappingErrors},
                           {METRIC_TOPOLOGY_ERRORS, TopologyError},
                           {METRIC_VALIDATION_ERRORS, ValidationErrors},
                           {METRIC_PIXEL_ERRORS, PixelErrors},
                           {METRIC_READOUT_TYPE_ERRORS, TypeErrors}}) {}
  };

  ///
  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param MaxRing Maximum valid Ring value (exclusive)
  /// \param MaxFEN Maximum valid FEN value (exclusive)
  /// \param GeomType Geometry type for runtime validation
  ///
  DetectorGeometry(Statistics &Stats, int MaxRing, int MaxFEN)
      : MaxRing(MaxRing), MaxFEN(MaxFEN),
        BaseCounters(Stats) {}

  /// \brief Virtual destructor for proper inheritance
  virtual ~DetectorGeometry() = default;

  /// \brief Public access to statistics counters
  mutable BaseGeometryCounters BaseCounters;

  /// \brief Pure virtual implementation method for pixel calculation
  /// Derived classes must implement this method for their specific geometry
  /// \param Data Pointer to readout data object (const, cast to appropriate
  /// type internally) \return Calculated pixel ID, or 0 if calculation failed
  virtual uint32_t calcPixelImpl(const void *Data) const = 0;

  /// \brief Pure virtual method for runtime type validation
  /// Derived classes must implement this method to validate readout data types
  /// specific to their geometry type (e.g., CAEN, VMM3, etc.)
  /// \param type_info Type information from typeid()
  /// \return true if type is valid for this geometry, false otherwise
  virtual bool inline validateDataType(const std::type_info &type_info) const = 0;

public:
  /// \brief Get access to BaseGeometryCounters object
  const BaseGeometryCounters &getBaseCounters() const { return BaseCounters; }

  /// \brief Validate ring number against configuration
  bool inline validateRing(int Ring) const {
    if (Ring < 0 || Ring > MaxRing) {
      XTRACE(DATA, WAR, "RING %d is invalid (out of range)", Ring);
      BaseCounters.RingErrors++;
      return false;
    }
    return true;
  }

  /// \brief Validate FEN number against configuration
  bool inline validateFEN(int FEN) const {
    if (FEN < 0 || FEN > MaxFEN) {
      XTRACE(DATA, WAR, "FEN %d is invalid (out of range)", FEN);
      BaseCounters.FENErrors++;
      return false;
    }
    return true;
  }

  template <typename T>
  bool validateTopology(HashMap2D<T> &map, int Col, int Row) const {
    if (not map.isValue(Col, Row)) {
      XTRACE(DATA, WAR, "Col %d, Row %d is incompatible with config", Col, Row);
      BaseCounters.TopologyError++;
      return false;
    }
    return true;
  }

  /// \brief Validate a list of boolean functions
  /// \param validators List of functions returning bool
  /// \return true if all validators return true, false otherwise
  template <typename... Validators>
  bool validateAll(Validators &&...validators) const {
    bool result = (validators() && ...);
    if (!result) {
      BaseCounters.ValidationErrors++;
    }
    return result;
  }

  /// \brief Template wrapper for pixel calculation with runtime type validation
  /// \tparam T The readout data type
  /// \param Readout Data object to calculate pixel for (const reference)
  /// \return Calculated pixel ID, with automatic error counting for failures
  template <typename T> uint32_t calcPixel(const T &Data) const {
    // Runtime type validation
    if (!validateDataType(typeid(T))) {
      XTRACE(DATA, ERR, "Invalid readout type for geometry type %s",
             typeid(T).name());
      BaseCounters.TypeErrors++;
      return 0;
    }

    // Use type erasure to call the derived class's calcPixelImpl method
    uint32_t pixel = calcPixelImpl(&Data);
    if (pixel == 0) {
      BaseCounters.PixelErrors++;
      XTRACE(DATA, DEB, "Pixel calculation failed, counted as pixel error");
    }

    return pixel;
  }
};

} // namespace geometry