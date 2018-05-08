/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX timestemp interpretation
 */

#pragma once

#include <inttypes.h>
#include <string>

class SRSTime {
  static const uint16_t internal_SRS_clock_ {40};

public:
  // setters
  void set_rebin_tdc(bool rebin_tdc);
  void set_bc_clock(double bc_clock);
  void set_tac_slope(double tac_slope);
  void set_trigger_resolution(double trigger_resolution);
  void set_target_resolution(double target_resolution);
  void set_acquisition_window(uint16_t acq_win);

  // getters
  bool rebin_tdc() const;
  double bc_clock() const;
  double tac_slope() const;
  double trigger_resolution() const;
  double timestamp_ns(int trigger_timestamp) const;
  uint16_t acquisition_window() const;

  double max_bcid_in_window() const;

  double delta_timestamp_ns(double old_timestamp_ns, double timestamp_ns,
                            unsigned int old_framecounter, unsigned int framecounter,
                            uint64_t &stats_triggertime_wraps) const;

  double trigger_period() const;

  double target_resolution() const;

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

  double chip_time(uint16_t bc, uint16_t tdc) const;

  // @brief prints out time configuration
  std::string debug() const;

private:
  bool rebin_tdc_{true};            // rebin tdc (for VMM3 bug)
  double bc_clock_{40};              // bc clock divisor
  double tac_slope_{125};            // tdc clock divisor

  double trigger_resolution_ {3.125}; // resolution of trigger timestamp in ns

  double target_resolution_ns_{0.5}; // target resolution for integer-valued timestamp

  uint16_t acquisition_window_ {4000};

  uint32_t recent_trigger_{0};
  uint64_t bonus_{0};

  double max_bcid_in_window_;
};
