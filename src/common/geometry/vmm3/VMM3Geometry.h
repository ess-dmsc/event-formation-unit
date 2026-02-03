// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common VMM3 geometry base class
///
/// Mapping from digital identifiers to x- and y- coordinates for VMM3-based
/// detectors. This class provides shared functionality for all VMM3 detectors
/// including Freia, NMX, and others.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/readout/vmm3/VMM3Config.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/Event.h>
#include <cstdint>
#include <limits>
#include <logical_geometry/ESSGeometry.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace vmm3 {

/// \brief Abstract base for VMM3-based geometries with ESSGeometry
/// inheritance. Concrete geometries inherit from this class.
/// Uses VMM3Parser::VMM3Data for validation and Event for pixel calculation.
class VMM3Geometry : public geometry::DetectorGeometry<VMM3Parser::VMM3Data, Event>,
                     public ESSGeometry {
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

  virtual ~VMM3Geometry() = default;

  /// \brief Check if the given VMM belongs to the X coordinate plane.
  ///
  /// Default policy: X on odd VMM, Y on even VMM. Instruments can override.
  /// \param VMM ASIC index (0..N) within the hybrid
  /// \return true if VMM contributes to X
  virtual inline bool isXCoord(uint8_t VMM) const { return (VMM & 0x1) != 0; }

  /// \brief Check if the given VMM belongs to the Y coordinate plane.
  ///
  /// Uses the complement of isXCoord() and thus follows overridden policies.
  /// \param VMM ASIC index (0..N) within the hybrid
  /// \return true if VMM contributes to Y
  virtual inline bool isYCoord(uint8_t VMM) const { return not isXCoord(VMM); }

  /// \brief Get access to the geometry counters for monitoring
  /// \return Const reference to VmmGeometryCounters instance
  const VmmGeometryCounters &getVmmCounters() const { return Counters; }

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
  [[nodiscard]] CoordResult calculateCoordinate(uint16_t XOffset, uint16_t YOffset,
                                                uint8_t VMM, uint8_t Channel);

  /// \brief Extract the raw 10-bit ADC value from OTADC field
  ///
  /// Common method for all VMM3-based instruments to consistently extract
  /// the valid 10-bit ADC value from the raw readout OTADC field.
  /// This masks out bits beyond the 10-bit ADC value.
  ///
  /// \param OTADC Raw OTADC field from VMM3 readout (typically 16-bit)
  /// \return 10-bit ADC value (range 0-1023)
  static inline uint16_t getRawADC(uint16_t OTADC) { return OTADC & 0x03FF; }

  /// \brief Calculate HybridId from VMM index
  /// \param VMM ASIC index
  /// \return HybridId uint8_t (VMM >> 1)
  static inline uint8_t calcHybridId(uint8_t VMM) { return VMM >> 1; }

protected:
  /// \brief Validate channel range based on VMM plane and coordinate type
  /// \param VMM ASIC index (determines plane)
  /// \param Channel Channel within the ASIC
  /// \return true if channel is valid for this plane's coordinate type
  bool validateChannel(uint8_t VMM, uint8_t Channel) const;

  /// \brief Protected constructor - only derived classes can create instances
  VMM3Geometry(Statistics &Stats, VMM3Config &Cfg, int MaxRing, int MaxFEN,
               int nx, int ny, int nz, int np)
      : DetectorGeometry(Stats, MaxRing, MaxFEN), ESSGeometry(nx, ny, nz, np),
        Counters(Stats), Conf(Cfg) {}

  // Methods that derived classes may need to override or call
  /// \brief Determine if this instrument uses wires for X plane (default:
  /// false, strips)
  virtual inline bool usesWiresForX() const { return false; }

  /// \brief Determine if this instrument uses wires for Y plane (default: true,
  /// wires)
  virtual inline bool usesWiresForY() const { return true; }

  /// \brief Unified calculation for strip-based coordinates
  /// \param Offset Instrument offset to add (for strips, typically YOffset but
  /// can be XOffset for Estia) \param Channel Channel within ASIC \return
  /// Calculated coordinate (Offset + NumStrips - 1 - Channel) or InvalidCoord
  /// if overflow
  [[nodiscard]] inline uint16_t calcFromStrip(uint16_t Offset, uint8_t Channel) const {
    // Use uint32_t to detect overflow
    uint32_t result = static_cast<uint32_t>(Offset) + NumStrips - 1 - Channel;
    XTRACE(DATA, INF, "Calculated strip coordinate: %u (Offset %u, Channel %u)",
           result, Offset, Channel);

    if (result > std::numeric_limits<uint16_t>::max()) {
      XTRACE(DATA, WAR, "Coordinate overflow: %u + %u - 1 - %u = %u", Offset,
             NumStrips, Channel, result);
      Counters.CoordOverflow++;
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
  [[nodiscard]] virtual uint16_t calcFromWire(uint16_t Offset, uint8_t Channel) const {
    // Use uint32_t to detect overflow
    uint32_t result = static_cast<uint32_t>(Offset) + MaxWireChannel - Channel;
    XTRACE(DATA, INF, "Calculated wire coordinate: %u (Offset %u, Channel %u)",
           result, Offset, Channel);

    if (result > std::numeric_limits<uint16_t>::max()) {
      XTRACE(DATA, WAR, "Coordinate overflow: %u + %u - %u = %u", Offset,
             MaxWireChannel, Channel, result);
      Counters.CoordOverflow++;
      return InvalidCoord;
    }
    return static_cast<uint16_t>(result);
  }

  /// \brief Validate that the (Ring,FEN,Hybrid) exists in configuration.
  /// \param Ring Logical ring index (from FiberId/2)
  /// \param FENId Front-end index within the ring
  /// \param HybridId Hybrid index (VMM pair)
  /// \return true if the hybrid is initialised in config, false otherwise
  inline bool validateHybrid(uint8_t Ring, uint8_t FENId, uint8_t HybridId) const {
    auto &Hybrid = Conf.getHybrid(Ring, FENId, HybridId);
    if (!Hybrid.Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             Ring, FENId, (HybridId << 1));
      Counters.HybridMappingErrors++;
      return false;
    }
    return true;
  }

  // Protected member variables - accessible to derived classes
  mutable VmmGeometryCounters Counters; // auto-registered stats
  VMM3Config &Conf;                     // shared configuration reference

private:
  // Private methods - only used internally by this class
  /// \brief Increment the appropriate error counter based on coordinate plane
  /// \param isX true for X coordinate plane, false for Y coordinate plane
  inline void incrementErrorCounter(bool isX) const {
    if (isX) {
      Counters.InvalidXCoord++;
    } else {
      Counters.InvalidYCoord++;
    }
  }

  /// \brief Pixel calculation used by DetectorGeometry template.
  /// For VMM3-based implementations, uses Event's cluster center-of-mass
  /// for both X and Y coordinates, then maps via pixel2D().
  /// \param Data Const reference to Event object
  /// \return Pixel id (0 if invalid per ESSGeometry)
  uint32_t calcPixelImpl(const Event &Data) const override {
    auto x = static_cast<uint16_t>(std::round(Data.ClusterA.coordCenter()));
    auto y = static_cast<uint16_t>(std::round(Data.ClusterB.coordCenter()));

    return ESSGeometry::pixel2D(x, y);
  }
};

} // namespace vmm3