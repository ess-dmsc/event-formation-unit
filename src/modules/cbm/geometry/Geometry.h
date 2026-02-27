// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unified CBM geometry class using topology-based validation
///
/// This class provides a single geometry object that uses the topology
/// configuration directly for validation and pixel calculation. No sub-modules
/// are needed - all information comes from the cached topology data.
///
/// Inherits from DetectorGeometry which manages base counters for:
/// - Ring/FEN/Topology validation errors
/// - Validation errors (general)
/// - Pixel calculation errors
///
/// This class adds CBM-specific counters for:
/// - MonitorRing mismatch errors
/// - Type mismatch errors
/// - X/Y position errors (2D monitors)
///
/// Cache optimization:
/// A compact CachedTopology struct is used instead of accessing the full
/// Topology objects, providing better cache locality for hot-path operations.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <cstdint>
#include <logical_geometry/ESSGeometry.h>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>

#include <unordered_map>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

using namespace geometry;

///
/// \brief Unified CBM geometry class using topology-based validation
///
/// This class manages all CBM beam monitor geometries using cached topology
/// data. No sub-modules are used - validation and pixel calculation are
/// performed directly based on cached topology information.
///
/// The class handles:
/// - Base counters via DetectorGeometry (Ring, FEN, Topology, Validation,
/// Pixel)
/// - CBM-specific counters (MonitorRing, Type, XPos, YPos)
/// - Common validation (Ring, FEN, MonitorRing, Type, Topology)
/// - Type-specific validation (X/Y bounds for 2D)
/// - Pixel calculation based on type
///
class Geometry : public DetectorGeometry<Parser::CbmReadout> {
public:
  // clang-format off
  // CBM-specific metric name constants (base metrics in DetectorGeometry)
  static inline const std::string_view METRIC_MONITOR_RING_MISMATCH_ERRORS{"geometry.monitor_ring_mismatch_errors"};
  static inline const std::string_view METRIC_TYPE_MISMATCH_ERRORS{"geometry.type_mismatch_errors"};
  static inline const std::string_view METRIC_XPOS_ERRORS{"geometry.xpos_errors"};
  static inline const std::string_view METRIC_YPOS_ERRORS{"geometry.ypos_errors"};
  // clang-format on

  ///
  /// \brief CBM-specific geometry statistics counters
  ///
  /// These counters are specific to CBM and supplement the base counters
  /// from DetectorGeometry.
  ///
  struct CbmGeometryCounters : public StatCounterBase {
    int64_t MonitorRingMismatchErrors{0}; ///< Ring != configured MonitorRing
    int64_t TypeMismatchErrors{0};        ///< Type != configured Type
    int64_t XPosErrors{0};                ///< X position out of range (2D)
    int64_t YPosErrors{0};                ///< Y position out of range (2D)

    explicit CbmGeometryCounters(Statistics &Stats)
        : StatCounterBase(
              Stats,
              {{METRIC_MONITOR_RING_MISMATCH_ERRORS, MonitorRingMismatchErrors},
               {METRIC_TYPE_MISMATCH_ERRORS, TypeMismatchErrors},
               {METRIC_XPOS_ERRORS, XPosErrors},
               {METRIC_YPOS_ERRORS, YPosErrors}}) {}
  };

  ///
  /// \brief Compact cached topology data for cache-optimized access
  ///
  /// This struct contains only the fields needed for validation and pixel
  /// calculation. ESSGeometry is always present; non-2D monitors use
  /// dummy values (0, 0, 1, 1).
  ///
  struct CachedTopology {
    CbmType Type; ///< Monitor type (EVENT_2D, EVENT_0D, IBM)
    ESSGeometry
        ESSGeom; ///< ESSGeometry for pixel calculations (dummy for non-2D)
    int PixelOffset{0}; ///< Pixel offset for 0D and optional 2D offset

    CachedTopology(CbmType type, int width, int height, int offset = 0)
        : Type(type), ESSGeom(width, height, 1, 1), PixelOffset(offset) {}

    CachedTopology(int pixelOffset)
        : Type(CbmType::EVENT_0D), ESSGeom(0, 0, 0, 0),
          PixelOffset(pixelOffset) {}

    CachedTopology()
        : Type(CbmType::IBM), ESSGeom(0, 0, 0, 0), PixelOffset(0) {}
  };

  ///
  /// \brief Constructor - initializes geometry from configuration
  ///
  /// Builds a cache of topology data for efficient validation and pixel
  /// calculation.
  ///
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Conf Reference to CBM configuration
  ///
  Geometry(Statistics &Stats, const Config &Conf)
      : DetectorGeometry(Stats, Conf.CbmParms.MaxRing, Conf.CbmParms.MaxFENId),
        CbmCounters(Stats), Conf(Conf), MonitorRing(Conf.CbmParms.MonitorRing) {
    initializeCache();
  }

  /// \brief Virtual destructor for proper cleanup
  virtual ~Geometry() = default;

  ///
  /// \brief Validate readout data using cached topology
  ///
  /// Performs common validation using validateAll pattern (like DREAM):
  /// 1. Ring validation - checks if Ring is within [0, MaxRing]
  /// 2. MonitorRing validation - checks if Ring matches configured MonitorRing
  /// 3. FEN validation - checks if FENId is within [0, MaxFEN]
  /// 4. Topology validation - verifies FEN/Channel exists in configuration
  /// 5. Type validation - checks if monitor type matches configuration
  /// 6. Type-specific validation - checks X/Y bounds for 2D monitors
  ///
  /// \param Data CbmReadout to validate
  /// \return true if all validations pass, false otherwise
  ///
  bool validateReadoutData(const Parser::CbmReadout &Data) const override {
    int Ring = calcRing(Data.FiberId);

    // Optimized lookup on cached topology (nullptr if not found)
    const auto *cached = getCachedTopology(Data.FENId, Data.Channel);

    // All validations using validateAll (handles ValidationErrors counting)
    return validateAll(
        [&]() { return validateRing(Ring); },
        [&]() { return validateMonitorRing(Ring); },
        [&]() { return validateFEN(Data.FENId); },
        [&]() {
          return validateTopology(*Conf.TopologyMapPtr, Data.FENId,
                                  Data.Channel);
        },
        [&]() {
          if (cached == nullptr) {
            BaseCounters.TopologyError++;
            XTRACE(DATA, WAR, "No topology for FEN %d, Channel %d", Data.FENId,
                   Data.Channel);
            return false;
          }
          return true;
        },
        [&]() { return validateMonitorType(Data.Type, cached->Type); },
        [&]() { return validateTypeSpecificData(cached, Data); });
  }

  ///
  /// \brief Get cached topology for a specific FEN/Channel
  ///
  /// \param FENId FEN identifier
  /// \param Channel Channel identifier
  /// \return Pointer to cached topology, or nullptr if not found
  ///
  const CachedTopology *getCachedTopology(uint8_t FENId,
                                          uint8_t Channel) const {
    uint16_t key = calcCacheKey(FENId, Channel);
    auto it = TopologyCache.find(key);
    return (it != TopologyCache.end()) ? &it->second : nullptr;
  }

  ///
  /// \brief Get access to CBM-specific geometry counters
  ///
  const CbmGeometryCounters &getCbmCounters() const { return CbmCounters; }

private:
  ///
  /// \brief Implementation of pixel calculation using cached topology
  ///
  /// Calculates pixel based on monitor type:
  /// - EVENT_2D: uses ESSGeometry::pixel2D(XPos, YPos) + optional PixelOffset
  /// - EVENT_0D: pixel = PixelOffset
  /// - IBM: pixel = 0
  ///
  /// \param Data CbmReadout containing position/event data
  /// \return Calculated pixel ID, or 0 if calculation fails
  ///
  uint32_t calcPixelImpl(const Parser::CbmReadout &Data) const override {

    auto *cached = getCachedTopology(Data.FENId, Data.Channel);
    if (cached == nullptr) {
      // No topology found - pixel calculation fails (error already counted in
      // validation)
      XTRACE(DATA, DEB,
             "Pixel calculation failed: no topology for FEN %d, Channel %d",
             Data.FENId, Data.Channel);
      return 0;
    }

    switch (static_cast<uint8_t>(cached->Type)) {
    case static_cast<uint8_t>(CbmType::EVENT_2D): {
      auto pixelId = cached->ESSGeom.pixel2D(Data.Pos.XPos, Data.Pos.YPos);
      return pixelId ? pixelId + cached->PixelOffset : pixelId;
    }
    case static_cast<uint8_t>(CbmType::EVENT_0D):
      return cached->PixelOffset;
    case static_cast<uint8_t>(CbmType::IBM):
    default:
      return 0;
    }
  }

  ///
  /// \brief Calculate cache key from FEN and Channel
  ///
  static inline uint16_t calcCacheKey(uint8_t FENId, uint8_t Channel) {
    return (static_cast<uint16_t>(FENId) << 8) | Channel;
  }

  ///
  /// \brief Initialize topology cache from configuration
  ///
  void initializeCache() {
    if (not Conf.TopologyMapPtr) {
      return;
    }

    std::vector<Topology *> topologies = Conf.TopologyMapPtr->toValuesList();
    for (const auto *item : topologies) {
      uint16_t key = calcCacheKey(item->FEN, item->Channel);

      if (item->Type == CbmType::EVENT_2D) {
        TopologyCache.emplace(key,
                              CachedTopology{item->Type, item->width,
                                             item->height, item->pixelOffset});
      } else if (item->Type == CbmType::EVENT_0D) {
        TopologyCache.emplace(key, CachedTopology{item->pixelOffset});
      } else { // IBM
        TopologyCache.emplace(key, CachedTopology{});
      }
    }
  }

  ///
  /// \brief Validate MonitorRing (CBM-specific)
  ///
  inline bool validateMonitorRing(int Ring) const {
    if (Ring != MonitorRing) {
      XTRACE(DATA, WAR, "Ring %d does not match MonitorRing %d", Ring,
             MonitorRing);
      CbmCounters.MonitorRingMismatchErrors++;
      return false;
    }
    return true;
  }

  ///
  /// \brief Validate monitor type matches configuration (CBM-specific)
  ///
  inline bool validateMonitorType(int DataType, CbmType ConfigType) const {
    if (DataType != static_cast<uint8_t>(ConfigType)) {
      XTRACE(DATA, WAR, "Type %d does not match configured %d", DataType,
             static_cast<uint8_t>(ConfigType));
      CbmCounters.TypeMismatchErrors++;
      return false;
    }
    return true;
  }

  ///
  /// \brief Validate type-specific data using cached topology
  ///
  /// For 2D monitors, validates X/Y coordinates against ESSGeometry dimensions.
  /// For 0D and IBM monitors, always returns true (no type-specific
  /// validation).
  ///
  inline bool validateTypeSpecificData(const CachedTopology *cached,
                                       const Parser::CbmReadout &Data) const {
    if (cached->Type == CbmType::EVENT_2D) {
      // Validate X coordinate
      if (Data.Pos.XPos >= cached->ESSGeom.nx()) {
        XTRACE(DATA, WAR, "XPos %u exceeds width %u", Data.Pos.XPos,
               cached->ESSGeom.nx());
        CbmCounters.XPosErrors++;
        return false;
      }
      // Validate Y coordinate
      if (Data.Pos.YPos >= cached->ESSGeom.ny()) {
        XTRACE(DATA, WAR, "YPos %u exceeds height %u", Data.Pos.YPos,
               cached->ESSGeom.ny());
        CbmCounters.YPosErrors++;
        return false;
      }
    }
    // 0D and IBM have no type-specific validation
    return true;
  }

  /// CBM-specific statistics counters
  mutable CbmGeometryCounters CbmCounters;

  /// Configuration reference
  const Config &Conf;

  /// Configured monitor ring
  const uint8_t MonitorRing;

  /// Topology cache: key = (FEN << 8) | Channel -> compact topology data
  std::unordered_map<uint16_t, CachedTopology> TopologyCache;
}; // namespace cbm

} // namespace cbm
