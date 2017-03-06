/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once

#include <inttypes.h>

class Time {
public:
  void set_bc_clock(double bc_clock);
  void set_tac_slope(double tac_slope);
  void set_trigger_resolution(double trigger_resolution);
  void set_target_resolution(double target_resolution);

  double timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) const;
  uint64_t timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) const;

private:
  double bc_clock_{40};
  double tac_slope_{125};
  double trigger_resolution_{3.125};

  double target_resolution_ns_{0.5};
};
