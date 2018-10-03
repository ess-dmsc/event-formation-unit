/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSTime.h>
#include <sstream>

void SRSTime::set_bc_clock(double bc_clock) {
  bc_clock_MHz_ = bc_clock;
  bc_factor_ = us_to_ns / bc_clock_MHz_;
}

void SRSTime::set_tac_slope(double tac_slope) {
  tac_slope_ns_ = tac_slope;
  tdc_factor_ = tac_slope_ns_ / tdc_range;
}

void SRSTime::set_trigger_resolution_ns(double ns) {
  trigger_resolution_ns_ = ns;
}

void SRSTime::set_target_resolution_ns(double ns) {
  target_resolution_ns_ = ns;
}

void SRSTime::set_acquisition_window(uint16_t acq_win) {
  acquisition_window_ = acq_win;
  // from us to ns
  max_chip_time_in_window_ns_ =
      us_to_ns * acquisition_window_ / internal_SRS_clock_MHz;
}

double SRSTime::bc_clock() const {
  return bc_clock_MHz_;
}

double SRSTime::tac_slope() const {
  return tac_slope_ns_;
}

double SRSTime::trigger_resolution_ns() const {
  return trigger_resolution_ns_;
}

double SRSTime::target_resolution_ns() const {
  return target_resolution_ns_;
}

uint16_t SRSTime::acquisition_window() const {
  return acquisition_window_;
}

double SRSTime::max_chip_time_in_window_ns() const {
  return max_chip_time_in_window_ns_;
}

double SRSTime::trigger_timestamp_ns(uint64_t trigger_timestamp) const {
  return trigger_timestamp * trigger_resolution_ns_;
}

double SRSTime::trigger_period_ns() const
{
  return bc_range * bc_factor_;
}

///
double SRSTime::chip_time_ns(uint16_t bc, uint16_t tdc) const
{
  return  static_cast<double>(bc + 1) * bc_factor_ - static_cast<double>(tdc) * tdc_factor_;
}

/// \brief using calibration data - corrected by Doro 3. october 2018
/// \todo please someone verify this implementation :-)
double SRSTime::chip_time_ns(uint16_t bc, uint16_t tdc, float offset, float slope) const
{
  return static_cast<double>(bc) * bc_factor_ + (bc_factor_ - static_cast<double>(tdc) * tdc_factor_  - offset)*slope;
}

double SRSTime::timestamp_ns(uint64_t trigger, uint16_t bc, uint16_t tdc) {
  return trigger_timestamp_ns(trigger) + chip_time_ns(bc, tdc);
}

uint64_t SRSTime::timestamp(uint64_t trigger, uint16_t bc, uint16_t tdc) {
  return static_cast<uint64_t>(timestamp_ns(trigger, bc, tdc)
      * target_resolution_ns_);
}

uint32_t SRSTime::internal_clock_period_ns() const
{
	return internal_SRS_clock_period_ns;
}

std::string SRSTime::debug() const {
  std::stringstream ss;
  ss << "    Chip time = (bc+1)*1000/" << bc_clock_MHz_ << " - tdc*" << tac_slope_ns_ << "/255 (ns)\n";
  ss << "    Trigger time = " << trigger_resolution_ns_ << "*trigger (ns)\n";
  ss << "    Maximum chip time in window = " << max_chip_time_in_window_ns_ << " (ns)\n";
  ss << "    Target resolution = " << target_resolution_ns_ << "  (ns)\n";
  return ss.str();
}
