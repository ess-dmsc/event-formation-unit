/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Time.h>

void Time::set_bc_clock(double bc_clock) { bc_clock_ = bc_clock; }

void Time::set_tac_slope(double tac_slope) { tac_slope_ = tac_slope; }

void Time::set_trigger_resolution(double trigger_resolution) {
  trigger_resolution_ = trigger_resolution;
}

void Time::set_target_resolution(double target_resolution) {
  target_resolution_ns_ = target_resolution;
}

double Time::bc_clock() const { return bc_clock_; }

double Time::tac_slope() const { return tac_slope_; }

double Time::trigger_resolution() const { return trigger_resolution_; }

double Time::target_resolution() const { return target_resolution_ns_; }

double Time::timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) const {
  // BC time: bcid value * 1/(clock frequency)
  double bcTime = double(bc) / bc_clock_;
  // TDC time: tacSlope * tdc value (8 bit) * ramp length
  double tdcTime = double(tdc) * tac_slope_ / 256.0;
  // Chip time: bcid plus tdc value
  double chip_time = bcTime * 1000 + tdcTime;

  double trigger_timestamp_ns = trigger * trigger_resolution_;
  return trigger_timestamp_ns + chip_time;
}

uint64_t Time::timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) const {
  return static_cast<uint64_t>(timestamp_ns(trigger, bc, tdc) *
                               target_resolution_ns_);
}
