/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/SRSTime.h>

void SRSTime::set_bc_clock(double bc_clock) { bc_clock_ = bc_clock; }

void SRSTime::set_tac_slope(double tac_slope) { tac_slope_ = tac_slope; }

void SRSTime::set_trigger_resolution(double trigger_resolution) {
  trigger_resolution_ = trigger_resolution;
}

void SRSTime::set_target_resolution(double target_resolution) {
  target_resolution_ns_ = target_resolution;
}

double SRSTime::bc_clock() const { return bc_clock_; }

double SRSTime::tac_slope() const { return tac_slope_; }

double SRSTime::trigger_resolution() const { return trigger_resolution_; }

double SRSTime::target_resolution() const { return target_resolution_ns_; }

double SRSTime::timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) {
  if (trigger < recent_trigger_)
      bonus_++;
  recent_trigger_ = trigger;

  // BC time: bcid value * 1/(clock frequency)
  double bcSRSTime = double(bc) / bc_clock_;
  // TDC time: tacSlope * tdc value (8 bit) * ramp length
  double tdcSRSTime = double(tdc) * tac_slope_ / 256.0;
  // Chip time: bcid plus tdc value
  double chip_time = bcSRSTime * 1000 + tdcSRSTime;

  double trigger_timestamp_ns = trigger * trigger_resolution_;
  return (trigger_timestamp_ns + chip_time) + (bonus_ << 32);
}

uint64_t SRSTime::timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) {
  return static_cast<uint64_t>(timestamp_ns(trigger, bc, tdc) *
                               target_resolution_ns_);
}
