/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX timestemp interpretation
 */

#pragma once

#include <inttypes.h>

class Time {
public:
  // setters
  void set_bc_clock(double bc_clock);
  void set_tac_slope(double tac_slope);
  void set_trigger_resolution(double trigger_resolution);
  void set_target_resolution(double target_resolution);

  // getters
  double bc_clock() const;
  double tac_slope() const;
  double trigger_resolution() const;
  double target_resolution() const;

  /** @brief generate absolute timestamp in nanoseconds
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  double timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) const;

  /** @brief generate absolute integer-valued timestamp
   * @param trigger trigger timestamp from SRS header
   * @param bc bunc crossing ID from VMM
   * @param tdc tdc value from VMM
   */
  uint64_t timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) const;

private:
  double bc_clock_{40};              // bc clock divisor
  double tac_slope_{125};            // tdc clock divisor
  double trigger_resolution_{3.125}; // resolution of trigger timestamp in ns

  double target_resolution_ns_{
      0.5}; // target resolution for integer-valued timestamp
};
