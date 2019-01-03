/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSTime.h>
#include <fmt/format.h>

namespace Gem {

void SRSTime::bc_clock_MHz(uint64_t bc_clock) {
  bc_clock_MHz_ = bc_clock;
  bc_factor_ = us_to_ns / bc_clock_MHz_;
  trigger_period_ns_ =  bc_range * bc_factor_;
}

void SRSTime::tac_slope_ns(uint64_t tac_slope) {
  tac_slope_ns_ = tac_slope;
  tdc_factor_ = static_cast<double>(tac_slope_ns_) / static_cast<double>(tdc_range);
}

void SRSTime::trigger_resolution_ns(double ns) {
  trigger_resolution_ns_ = ns;
}

void SRSTime::acquisition_window(uint16_t acq_win) {
  acquisition_window_ = acq_win;
  //from us to ns
  max_chip_time_in_window_ns_ =
      acquisition_window_ * us_to_ns / internal_SRS_clock_MHz;
}

uint64_t SRSTime::bc_clock_MHz() const {
  return bc_clock_MHz_;
}

uint64_t SRSTime::tac_slope_ns() const {
  return tac_slope_ns_;
}

double SRSTime::trigger_resolution_ns() const {
  return trigger_resolution_ns_;
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

uint64_t SRSTime::trigger_period_ns() const {
  return trigger_period_ns_;
}

/// \brief using calibration data - corrected by Doro 3. october 2018
/// \todo please someone verify this implementation :-)
double SRSTime::chip_time_ns(uint16_t bc, uint16_t tdc, float offset, float slope) const {
  return static_cast<double>(bc) * static_cast<double>(bc_factor_) +
      (static_cast<double>(bc_factor_) - static_cast<double>(tdc) * tdc_factor_ -
          static_cast<double>(offset)) * static_cast<double>(slope);
}

std::string SRSTime::debug() const {
  std::string ret;

  // \todo does this reflect the function above?

  ret += fmt::format("  Chip time = {} * bc + ({} - {}*tdc - cal_offset) * cal_slope\n",
      bc_factor_, bc_factor_, tdc_factor_);
  ret += fmt::format("  Trigger time = {}*trigger (ns)\n", trigger_resolution_ns_);
  ret += fmt::format("  Maximum chip time in window = {} (ns)\n", max_chip_time_in_window_ns_);

  return ret;
}

}
