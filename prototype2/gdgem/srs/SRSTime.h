/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX timestemp interpretation
 */

#pragma once

#include <inttypes.h>
#include <string>
#include <limits>

class SRSTime {
  static constexpr double us_to_ns {1000};
  static constexpr double internal_SRS_clock_mhz {40};
  static constexpr double bc_resolution {4095};
  static constexpr double tdc_resolution {255};
  static constexpr double trigger_resolution {std::numeric_limits<uint32_t>::max()};

public:
  // setters
  void set_rebin_tdc(bool rebin_tdc);
  void set_bc_clock(double bc_clock);
  void set_tac_slope(double tac_slope);
  void set_trigger_resolution_ns(double ns);
  void set_target_resolution_ns(double ns);
  void set_acquisition_window(uint16_t acq_win);

  // getters
  bool rebin_tdc() const;
  double bc_clock() const;
  double tac_slope() const;
  double trigger_resolution_ns() const;
  double trigger_timestamp_ns(uint32_t trigger_timestamp) const;
  uint16_t acquisition_window() const;

  double max_chip_time_in_window_ns() const;

  double delta_timestamp_ns(double old_timestamp_ns, double timestamp_ns,
                            uint32_t old_framecounter, uint32_t framecounter,
                            size_t &stats_triggertime_wraps) const;

  double trigger_period_ns() const;

  double target_resolution_ns() const;

  /** @brief generate absolute timestamp in nanoseconds
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  double timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc);

  /** @brief generate absolute integer-valued timestamp
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  uint64_t timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc);

  double chip_time_ns(uint16_t bc, uint16_t tdc) const;

  // @brief prints out time configuration
  std::string debug() const;

private:
  bool rebin_tdc_{true};            // rebin tdc (for VMM3 bug)
  double bc_clock_{40};              // bc clock divisor TODO units
  double tac_slope_{125};            // tdc clock divisor TODO units
  double trigger_resolution_ns_ {3.125}; // resolution of trigger timestamp in ns
  double target_resolution_ns_ {0.5}; // target resolution for integer-valued timestamp
  uint16_t acquisition_window_{4000}; // TODO units

  // mutable for sequential ops
  uint32_t recent_trigger_{0};
  uint64_t bonus_{0};

  //precalculated
  double max_chip_time_in_window_ns_;
};