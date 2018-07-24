/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
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

//TODO Split away chip timing class

class SRSTime {
  static constexpr double us_to_ns {1000};
  static constexpr double internal_SRS_clock_MHz {40};
  static constexpr double internal_SRS_clock_period_ns {25};
  static constexpr double bc_range {4095};
  static constexpr double tdc_range {255};

  public:
  // setters  TODO reflect units
  void set_bc_clock(double bc_clock);
  void set_tac_slope(double tac_slope);
  void set_trigger_resolution_ns(double ns);
  void set_target_resolution_ns(double ns);
  void set_acquisition_window(uint16_t acq_win);

  // getters
  double bc_clock() const;
  double tac_slope() const;
  double trigger_resolution_ns() const;
  double trigger_timestamp_ns(uint64_t trigger_timestamp) const;
  uint16_t acquisition_window() const;

  double max_chip_time_in_window_ns() const;

  double trigger_period_ns() const;

  double target_resolution_ns() const;

  uint32_t internal_clock_period_ns() const;

  /** \brief generate absolute timestamp in nanoseconds
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  double timestamp_ns(uint64_t trigger, uint16_t bc, uint16_t tdc);

  /** \brief generate absolute integer-valued timestamp
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  uint64_t timestamp(uint64_t trigger, uint16_t bc, uint16_t tdc);

  double chip_time_ns(uint16_t bc, uint16_t tdc) const;

  // \brief prints out time configuration
  std::string debug() const;

private:
  double bc_clock_MHz_{40};              // bc clock divisor
  double tac_slope_ns_{125};            // tdc clock divisor
  double trigger_resolution_ns_ {3.125}; // resolution of trigger timestamp in ns
  double target_resolution_ns_ {0.5}; // target resolution for integer-valued timestamp
  uint16_t acquisition_window_{4000}; // unitless (divided later by MHz)

  //precalculated
  double max_chip_time_in_window_ns_{0};
  double tdc_factor_{1};
  double bc_factor_{1};
};
