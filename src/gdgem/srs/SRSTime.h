/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for NMX timestemp interpretation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <inttypes.h>
#include <string>
#include <limits>

/// \todo Split away chip timing class

namespace Gem {

class SRSTime {
public:
  static constexpr uint64_t us_to_ns{1000};
  static constexpr double internal_SRS_clock_MHz{40};
  static constexpr uint64_t internal_SRS_clock_period_ns {25};
  static constexpr uint64_t bc_range{4096};
  static constexpr uint64_t tdc_range{255};

  // setters
  void bc_clock_MHz(uint64_t bc_clock);
  void tac_slope_ns(uint64_t tac_slope);
  void trigger_resolution_ns(double ns);
  void acquisition_window(uint16_t acq_win);

  // getters
  uint64_t bc_clock_MHz() const;
  uint64_t tac_slope_ns() const;
  double trigger_resolution_ns() const;
  double trigger_timestamp_ns(uint64_t trigger_timestamp) const;
  uint16_t acquisition_window() const;

  // \todo is this still needed?
  double max_chip_time_in_window_ns() const;

  uint64_t trigger_period_ns() const;

  /// \brief now using VMM calibration data
  double chip_time_ns(uint16_t bc, uint16_t tdc, float offset, float slope) const;

  /// \brief prints out time configuration
  std::string debug() const;

private:
  uint64_t bc_clock_MHz_{20};              /// bc clock divisor
  uint64_t bc_factor_{50};
  uint64_t trigger_period_ns_ {bc_range * 50};

  uint64_t tac_slope_ns_{100};            /// tdc clock divisor
  double tdc_factor_{1};

  double trigger_resolution_ns_{1.0}; /// resolution of trigger timestamp in ns
  uint16_t acquisition_window_{4095}; /// unitless (divided later by MHz)

  //precalculated
  double max_chip_time_in_window_ns_{0};
};

}
