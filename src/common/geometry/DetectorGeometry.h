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

namespace geometry {

///
/// \brief Common detector geometry base class for ESS Ring/FEN detectors
///
/// This class provides shared functionality for detectors that use the ESS
/// Ring/FEN readout structure. It automatically registers consistent counters
/// and provides common validation methods to eliminate code duplication.
///
/// Usage:
/// 1. Inherit from DetectorGeometry<TReadout, TData> where:
///    - TReadout is the readout data type for validation
///    - TData is the data type for pixel calculation
/// 2. Implement calcPixelImpl(const TData &Data) for your specific geometry
/// 3. Implement validateReadoutData(const TReadout &Data) for validation
/// 4. Call calcPixel(Data) to get automatic error counting
/// 5. Error counters are automatically updated and registered
///
/// For detectors using the same type for both, use DetectorGeometry<T, T>
///
template <typename TReadout, typename TData = TReadout> class DetectorGeometry {
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
  // clang-format on

protected:
  const int MaxRing{0}; ///< Maximum valid Ring value (exclusive)
  const int MaxFEN{0};  ///< Maximum valid FEN value (exclusive)

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
                           {METRIC_PIXEL_ERRORS, PixelErrors}}) {}
  };

  ///
  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param MaxRing Maximum valid Ring value (exclusive)
  /// \param MaxFEN Maximum valid FEN value (exclusive)
  /// \param GeomType Geometry type for runtime validation
  ///
  DetectorGeometry(Statistics &Stats, int MaxRing, int MaxFEN)
      : MaxRing(MaxRing), MaxFEN(MaxFEN), BaseCounters(Stats) {}

public:
  /// \brief Virtual destructor for proper inheritance
  virtual ~DetectorGeometry() = default;

protected:
  /// \brief Public access to statistics counters
  mutable BaseGeometryCounters BaseCounters;

  /// \brief Pure virtual implementation method for pixel calculation
  /// Derived classes must implement this method for their specific geometry
  /// \param Data Const reference to readout data object of type TData
  /// \return Calculated pixel ID, or 0 if calculation failed
  virtual uint32_t calcPixelImpl(const TData &Data) const = 0;

  /// \brief Validate ring number against configuration
  inline bool validateRing(int Ring) const {
    if (Ring < 0 || Ring > MaxRing) {
      XTRACE(DATA, WAR, "RING %d is invalid (out of range)", Ring);
      BaseCounters.RingErrors++;
      return false;
    }
    return true;
  }

  /// \brief Validate FEN number against configuration
  inline bool validateFEN(int FEN) const {
    if (FEN < 0 || FEN > MaxFEN) {
      XTRACE(DATA, WAR, "FEN %d is invalid (out of range)", FEN);
      BaseCounters.FENErrors++;
      return false;
    }
    return true;
  }

  template <typename U>
  inline bool validateTopology(HashMap2D<U> &map, int Col, int Row) const {
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
  inline bool validateAll(Validators &&...validators) const {
    bool result = (validators() && ...);
    if (!result) {
      BaseCounters.ValidationErrors++;
    }
    return result;
  }

public:
  /// \brief Get access to BaseGeometryCounters object
  /// \return Const reference to BaseGeometryCounters instance
  inline const BaseGeometryCounters &getBaseCounters() const {
    return BaseCounters;
  }

  /// \brief Calculate Ring from FiberId using physical->logical mapping
  /// Common method used across all VMM3-based detectors.
  /// Ring = FiberId / 2 (each physical fiber pair maps to one logical ring)
  /// \param FiberId Physical fiber identifier
  /// \return Logical ring number
  static inline uint8_t calcRing(uint8_t FiberId) { return FiberId / 2; }

  /// \brief Validate readout data - must be implemented by derived classes
  /// \param Data Data object of type TReadout to validate
  /// \return true if validation passes, false otherwise
  virtual bool validateReadoutData(const TReadout &Data) const = 0;

  /// \brief Pixel calculation with automatic error counting
  /// Uses compile-time type checking via template parameter TData
  /// \param Data Data object of type TData to calculate pixel for
  /// \return Calculated pixel ID, with automatic error counting for failures
  inline uint32_t calcPixel(const TData &Data) const {
    // Use type erasure to call the derived class's calcPixelImpl method
    uint32_t pixel = calcPixelImpl(Data);
    if (pixel == 0) {
      BaseCounters.PixelErrors++;
      XTRACE(DATA, DEB, "Pixel calculation failed, counted as pixel error");
    }

    return pixel;
  }
};

} // namespace geometry