// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/Event.h>
#include <cstdint>
#include <freia/geometry/Config.h>
#include <limits>
#include <logical_geometry/ESSGeometry.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

// Forward declaration for Config
class Config;

/// \brief Abstract base for Freia family geometries with ESSGeometry
/// inheritance. Concrete geometries inherit from this class.
class GeometryBase : public geometry::DetectorGeometry, public ESSGeometry {
public:
  /// \brief Stats counters for geometry-level validation.
  struct VmmGeometryCounters : public StatCounterBase {
    int64_t HybridMappingErrors{0};
    int64_t InvalidXCoord{0};
    int64_t InvalidYCoord{0};
    int64_t CoordOverflow{0};
    int64_t StripChannelRangeErrors{0};
    int64_t WireChannelRangeErrors{0};

    // metric names
    // clang-format off
    static inline const std::string METRIC_HYBRID_MAPPING_ERRORS{"geometry.hybrid_mapping_errors"};
    static inline const std::string METRIC_INVALID_X_COORD_ERROR{"geometry.invalid_xcoord_error"};
    static inline const std::string METRIC_INVALID_Y_COORD_ERROR{"geometry.invalid_ycoord_error"};
    static inline const std::string METRIC_COORD_OVERFLOW_ERROR{"geometry.coord_overflow_error"};
    static inline const std::string METRIC_STRIP_VALIDATION_ERRORS{"geometry.strip_channel_range_errors"};
    static inline const std::string METRIC_WIRE_VALIDATION_ERRORS{"geometry.wire_channel_range_errors"};
    // clang-format on

    VmmGeometryCounters(Statistics &Stats)
        : StatCounterBase(
              Stats,
              {{METRIC_HYBRID_MAPPING_ERRORS, HybridMappingErrors},
               {METRIC_INVALID_X_COORD_ERROR, InvalidXCoord},
               {METRIC_INVALID_Y_COORD_ERROR, InvalidYCoord},
               {METRIC_COORD_OVERFLOW_ERROR, CoordOverflow},
               {METRIC_STRIP_VALIDATION_ERRORS, StripChannelRangeErrors},
               {METRIC_WIRE_VALIDATION_ERRORS, WireChannelRangeErrors}}) {}
  };

  /// \brief Result structure for coordinate calculations
  struct CoordResult {
    uint16_t coord;
    bool isXPlane;
  };

  // Static constants - public for external access
  static const uint16_t InvalidCoord;
  static const uint16_t NumStrips;
  static const uint16_t NumWires;
  static const uint16_t MinWireChannel;
  static const uint16_t MaxWireChannel;

  // Public interface methods
  /// \brief Check if the given VMM belongs to the X coordinate plane.
  ///
  /// Default policy: X on odd VMM, Y on even VMM. Instruments can override.
  /// \param VMM ASIC index (0..N) within the hybrid
  /// \return true if VMM contributes to X
  virtual bool isXCoord(uint8_t VMM) { return (VMM & 0x1) != 0; }

  /// \brief Check if the given VMM belongs to the Y coordinate plane.
  ///
  /// Uses the complement of isXCoord() and thus follows overridden policies.
  /// \param VMM ASIC index (0..N) within the hybrid
  /// \return true if VMM contributes to Y
  virtual bool isYCoord(uint8_t VMM) { return not isXCoord(VMM); }

  /// \brief Validate a raw VMM3 readout according to instrument geometry.
  ///
  /// Derived geometries should compose common checks using validateAll() and
  /// instrument-specific rules (ring/FEN/hybrid ranges etc.).
  /// \param Data VMM3 readout to validate
  /// \return true if valid, false otherwise (and increments counters)
  virtual bool
  validateReadoutData(const ESSReadout::VMM3Parser::VMM3Data &Data) = 0;

  /// \brief Runtime type validation for pixel calculation input.
  ///
  /// Current implementation accepts Event for calcPixelImpl path.
  /// \param type_info std::type_info of the provided data pointer
  /// \return true if type is the supported Event type, false otherwise
  bool validateDataType(const std::type_info &type_info) override {
    // For VMM3 geometries, we can accept either VMM3Parser::VMM3Data or Event
    XTRACE(DATA, DEB, "Validating data type: %s", type_info.name());
    if (type_info == typeid(Event)) {
      return true;
    } else {
      XTRACE(DATA, WAR, "Invalid data type for VMM3 geometry: %s",
             type_info.name());
      return false;
    }
  }

  /// \brief Get access to the geometry counters for monitoring
  const VmmGeometryCounters &getVmmGeometryCounters() const {
    return GeometryCounters;
  }

  /// \brief Unified coordinate calculation that determines plane and validates.
  ///
  /// This function automatically determines if the VMM belongs to X or Y plane,
  /// validates accordingly, and computes the coordinate. Reduces code
  /// duplication at call sites by handling the plane decision internally.
  ///
  /// \param XOffset X offset for this hybrid/cassette
  /// \param YOffset Y offset for this hybrid/cassette
  /// \param VMM ASIC index
  /// \param Channel Channel within the ASIC
  /// \return {coordinate, isXPlane} - coordinate is InvalidCoord on failure
  CoordResult calculateCoordinate(uint16_t XOffset, uint16_t YOffset,
                                  uint8_t VMM, uint8_t Channel);

  /// \brief Validate channel range based on VMM plane and coordinate type
  /// \param VMM ASIC index (determines plane)
  /// \param Channel Channel within the ASIC
  /// \return true if channel is valid for this plane's coordinate type
  bool validateChannel(uint8_t VMM, uint8_t Channel);

  virtual ~GeometryBase() = default;

protected:
  /// \brief Protected constructor - only derived classes can create instances
  GeometryBase(Statistics &Stats, Config &Cfg, int MaxRing, int MaxFEN, int nx,
               int ny, int nz, int np)
      : geometry::DetectorGeometry(Stats, MaxRing, MaxFEN,
                                   geometry::GeometryType::VMM3),
        ESSGeometry(nx, ny, nz, np), GeometryCounters(Stats), Conf(Cfg) {}

  // Methods that derived classes may need to override or call
  /// \brief Determine if this instrument uses wires for X plane (default:
  /// false, strips)
  virtual bool inline usesWiresForX() const { return false; }

  /// \brief Determine if this instrument uses wires for Y plane (default: true,
  /// wires)
  virtual bool inline usesWiresForY() const { return true; }

  /// \brief Unified calculation for strip-based coordinates
  /// \param Offset Instrument offset to add (for strips, typically YOffset but
  /// can be XOffset for Estia) \param Channel Channel within ASIC \return
  /// Calculated coordinate (Offset + NumStrips - 1 - Channel) or InvalidCoord
  /// if overflow
  inline uint16_t calcFromStrip(uint16_t Offset, uint8_t Channel) const {
    // Use uint32_t to detect overflow
    uint32_t result = static_cast<uint32_t>(Offset) + NumStrips - 1 - Channel;
    XTRACE(DATA, INF, "Calculated strip coordinate: %u (Offset %u, Channel %u)",
           result, Offset, Channel);

    if (result > std::numeric_limits<uint16_t>::max()) {
      XTRACE(DATA, WAR, "Coordinate overflow: %u + %u - 1 - %u = %u", Offset,
             NumStrips, Channel, result);
      GeometryCounters.CoordOverflow++;
      return InvalidCoord;
    }
    return static_cast<uint16_t>(result);
  }

  /// \brief Unified calculation for wire-based coordinates
  /// \param Offset Instrument offset to add
  /// \param Channel Channel within ASIC
  /// \return Calculated coordinate (Offset + MaxWireChannel - Channel) or
  /// InvalidCoord if overflow \note This function can be overridden by
  /// instruments like Estia
  virtual uint16_t calcFromWire(uint16_t Offset, uint8_t Channel) const {
    // Use uint32_t to detect overflow
    uint32_t result = static_cast<uint32_t>(Offset) + MaxWireChannel - Channel;
    XTRACE(DATA, INF, "Calculated wire coordinate: %u (Offset %u, Channel %u)",
           result, Offset, Channel);

    if (result > std::numeric_limits<uint16_t>::max()) {
      XTRACE(DATA, WAR, "Coordinate overflow: %u + %u - %u = %u", Offset,
             MaxWireChannel, Channel, result);
      GeometryCounters.CoordOverflow++;
      return InvalidCoord;
    }
    return static_cast<uint16_t>(result);
  }

  /// \brief Validate that the (Ring,FEN,Hybrid) exists in configuration.
  /// \param Ring Logical ring index (from FiberId/2)
  /// \param FENId Front-end index within the ring
  /// \param HybridId Hybrid index (VMM pair)
  /// \return true if the hybrid is initialised in config, false otherwise
  virtual bool validateHybrid(uint8_t Ring, uint8_t FENId, uint8_t HybridId);

  // Protected member variables - accessible to derived classes
  mutable VmmGeometryCounters GeometryCounters; // auto-registered stats
  Config &Conf; // shared configuration reference

private:
  // Private methods - only used internally by this class
  /// \brief Increment the appropriate error counter based on coordinate plane
  /// \param isX true for X coordinate plane, false for Y coordinate plane
  inline void incrementErrorCounter(bool isX) {
    if (isX) {
      GeometryCounters.InvalidXCoord++;
    } else {
      GeometryCounters.InvalidYCoord++;
    }
  }

  /// \brief Pixel calculation used by DetectorGeometry. For VMM3-based
  /// implementations, this function expects a pointer to Event. Uses cluster
  /// center-of-mass for both X and Y, then maps via pixel2D(). \param Data
  /// Pointer to Event \return Pixel id (0 if invalid per ESSGeometry)
  uint32_t calcPixelImpl(const void *Data) override {
    const auto &Event = *static_cast<const class Event *>(Data);

    auto x = static_cast<uint16_t>(std::round(Event.ClusterA.coordCenter()));
    auto y = static_cast<uint16_t>(std::round(Event.ClusterB.coordCenter()));

    return ESSGeometry::pixel2D(x, y);
  }
};

} // namespace Freia
