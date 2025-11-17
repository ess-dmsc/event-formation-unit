// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX geometry class with validation capabilities
/// Based on NMX ICD document
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/geometry/vmm3/VMM3Geometry.h>
#include <common/readout/vmm3/VMM3Parser.h>
#include <common/reduction/Event.h>
#include <logical_geometry/ESSGeometry.h>
#include <nmx/geometry/Config.h>

#include <cstdint>
#include <string>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Nmx {

// Forward declaration
class Config;

/// \brief NMX geometry class with DetectorGeometry and ESSGeometry inheritance
class NMXGeometry final : public vmm3::VMM3Geometry {
public:
  static constexpr uint32_t ESSGEOMETRY_NZ = 1;
  static constexpr uint32_t ESSGEOMETRY_NP = 1;

  /// \brief NMX-specific stats counters that extend VMM3 base counters
  struct NmxGeometryCounters : public StatCounterBase {
    int64_t ChannelRangeErrors{0};
    int64_t AsicRangeErrors{0};
    int64_t ADCErrors{0};

    NmxGeometryCounters(Statistics &Stats)
        : StatCounterBase(Stats,
                          {{METRIC_CHANNEL_RANGE_ERRORS, ChannelRangeErrors},
                           {METRIC_ASIC_RANGE_ERRORS, AsicRangeErrors},
                           {METRIC_ADC_ERRORS, ADCErrors}}) {}
  };

  // clang-format off
  static inline const std::string METRIC_CHANNEL_RANGE_ERRORS{"geometry.channel_range_errors"};
  static inline const std::string METRIC_ASIC_RANGE_ERRORS{"geometry.asic_range_errors"};
  static inline const std::string METRIC_ADC_ERRORS{"geometry.adc_errors"};
  // clang-format on

  /// \brief Constructor
  /// \param Stats Reference to Statistics object for counter registration
  /// \param Cfg Reference to NMX configuration object
  /// \param MaxRing Maximum ring number for validation
  /// \param MaxFEN Maximum FEN number for validation
  NMXGeometry(Statistics &Stats, Config &Cfg, int MaxRing, int MaxFEN)
      : vmm3::VMM3Geometry(
            Stats, Cfg, MaxRing, MaxFEN, Cfg.NMXFileParameters.SizeX,
            Cfg.NMXFileParameters.SizeY, ESSGEOMETRY_NZ, ESSGEOMETRY_NP),
        NmxCounters(Stats) {}

  /// \brief Validate VMM3 readout data for NMX geometry
  /// \param Data VMM3 readout data to validate
  /// \return true if readout is valid, false otherwise
  bool validateReadoutData(const vmm3::VMM3Parser::VMM3Data &Data) override;

  /// \brief Runtime type validation for pixel calculation input
  bool validateDataType(const std::type_info &type_info) const override {
    XTRACE(DATA, DEB, "Validating data type: %s", type_info.name());
    if (type_info == typeid(Event)) {
      return true;
    } else {
      XTRACE(DATA, WAR, "Invalid data type for NMX geometry: %s",
             type_info.name());
      return false;
    }
  }

  /// \brief Get access to the NMX-specific geometry counters for monitoring
  const NmxGeometryCounters &getNmxCounters() const { return NmxCounters; }

  /// \brief Extract ASIC ID from VMM number
  /// \param VMM VMM number (0..N)
  /// \return ASIC ID (0 or 1)
  static inline uint8_t getAsicId(uint8_t VMM) { return VMM & 0x1; }

  /// \brief Calculate coordinate value from channel, ASIC, offset and reversal
  /// flag \param Channel Channel number (0-63) \param AsicId ASIC identifier (0
  /// or 1) \param Offset Base offset for coordinate calculation \param
  /// ReversedChannels Whether channel order is reversed \return Calculated
  /// coordinate or InvalidCoord if invalid
  [[nodiscard]] uint16_t coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset,
                               bool ReversedChannels);

private:
  /// \brief Validate channel and ASIC ID ranges
  /// \param AsicId ASIC identifier (expected 0-1)
  /// \param Channel Channel within the ASIC (expected 0-63)
  /// \return true if both are valid, false otherwise
  inline bool validateAsicIdAndChannel(uint8_t AsicId, uint8_t Channel) {
    // NMX uses strip-based channels: 0-63 per ASIC (2 ASICs total)
    bool isValid = true;

    if (Channel >= NumStrips) {
      XTRACE(DATA, WAR, "Invalid Channel %d (expected 0-%d)", Channel,
             NumStrips - 1);
      NmxCounters.ChannelRangeErrors++;
      isValid = false;
    }

    if (AsicId >= 2) {
      XTRACE(DATA, WAR, "Invalid AsicId %d (expected 0-1)", AsicId);
      NmxCounters.AsicRangeErrors++;
      isValid = false;
    }

    return isValid;
  }

  /// \brief Validate ADC value against minimum threshold
  /// \param ADC ADC value to validate
  /// \param Ring Ring number for hybrid lookup
  /// \param FENId FEN identifier
  /// \param HybridId Hybrid identifier
  /// \return true if ADC is valid, false otherwise
  bool validateADC(uint16_t ADC, int Ring, uint8_t FENId,
                   uint8_t HybridId) const;

  // Protected member variables - accessible to derived classes
  mutable NmxGeometryCounters NmxCounters; // auto-registered NMX-specific stats

  /// \brief Pixel calculation used by DetectorGeometry
  /// \param Data Pointer to Event object
  /// \return Pixel id (0 if invalid per ESSGeometry)
  inline uint32_t calcPixelImpl(const void *Data) const override {
    const auto &Event = *static_cast<const class Event *>(Data);

    auto x = static_cast<uint16_t>(std::round(Event.ClusterA.coordUtpc(false)));
    auto y = static_cast<uint16_t>(std::round(Event.ClusterB.coordUtpc(false)));

    auto pixelId = ESSGeometry::pixel2D(x, y);

    XTRACE(EVENT, INF, "Calculating pixel for x: %u, y: %u, pixel: %u", x, y,
           pixelId);

    return pixelId;
  }
};

} // namespace Nmx
