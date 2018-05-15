/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSTime.h>
#include <sstream>

void SRSTime::set_rebin_tdc(bool rebin_tdc) {
  rebin_tdc_ = rebin_tdc;
}

void SRSTime::set_bc_clock(double bc_clock) {
  bc_clock_ = bc_clock;
}

void SRSTime::set_tac_slope(double tac_slope) {
  tac_slope_ = tac_slope;
}

void SRSTime::set_trigger_resolution_ns(double ns) {
  trigger_resolution_ns_ = ns;
}

void SRSTime::set_target_resolution_ns(double ns) {
  target_resolution_ns_ = ns;
}

void SRSTime::set_acquisition_window(uint16_t acq_win)
{
  acquisition_window_ = acq_win;
  // from us to ns
  max_chip_time_in_window_ns_ =
      us_to_ns * acquisition_window_ / internal_SRS_clock_mhz;
}

bool SRSTime::rebin_tdc() const {
  return rebin_tdc_;
}

double SRSTime::bc_clock() const {
  return bc_clock_;
}

double SRSTime::tac_slope() const {
  return tac_slope_;
}

double SRSTime::trigger_resolution_ns() const {
  return trigger_resolution_ns_;
}

double SRSTime::target_resolution_ns() const {
  return target_resolution_ns_;
}

uint16_t SRSTime::acquisition_window() const
{
  return acquisition_window_;
}

double SRSTime::max_chip_time_in_window_ns() const
{
  return max_chip_time_in_window_ns_;
}

double SRSTime::trigger_timestamp_ns(uint32_t trigger_timestamp) const {
  return trigger_timestamp * trigger_resolution_ns_;
}

double SRSTime::delta_timestamp_ns(double old_timestamp_ns,
                                   double timestamp_ns,
                                   uint32_t old_framecounter,
                                   uint32_t framecounter,
                                   size_t &stats_triggertime_wraps) const {
  if (old_timestamp_ns > timestamp_ns
      && (old_framecounter <= framecounter
          || old_framecounter > framecounter + 1000000000))
  {
    stats_triggertime_wraps++;
    return (trigger_resolution_ns_ * trigger_resolution
        + timestamp_ns - old_timestamp_ns);
  } else {
    return (timestamp_ns - old_timestamp_ns);
  }
}

double SRSTime::trigger_period_ns() const {
  return us_to_ns * bc_resolution / bc_clock_;
}

double SRSTime::chip_time_ns(uint16_t bc, uint16_t tdc) const {
  // Calculate bcTime [us]

  double bcTime = bc / bc_clock_;

  // TDC time: pTAC * tdc value (8 bit)/ramp length
  // [ns]

  // TODO: Test this logic
  // TDC has reduced resolution due to most significant bit problem of current
  // sources (like ADC)
  if (rebin_tdc_) {
    // TODO: use bit shifting instead?
    uint16_t tdcRebinned = (uint16_t) tdc / 8;
    tdc = tdcRebinned * 8;
  }

  double tdcTime = tac_slope_ * static_cast<double>(tdc) / tdc_resolution;

  // Chip time: bcid plus tdc value
  // Talk Vinnie: HIT time  = BCIDx25 + ADC*125/255 [ns]
  return bcTime * us_to_ns + tdcTime;
}

double SRSTime::timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) {
  if (trigger < recent_trigger_)
    bonus_++;
  recent_trigger_ = trigger;

  return (bonus_ << 32) +
      trigger_timestamp_ns(trigger) + chip_time_ns(bc, tdc);
}

uint64_t SRSTime::timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) {
  return static_cast<uint64_t>(timestamp_ns(trigger, bc, tdc)
      * target_resolution_ns_);
}

std::string SRSTime::debug() const {
  std::stringstream ss;
  ss << "    Chip time = bc*1000/" << bc_clock_ << " + tdc*" << tac_slope_ << "/255 (ns)\n";
  ss << "         Rebin TDC (for VMM3 bug) = " << (rebin_tdc_ ? "YES" : "no") << " (ns)\n";
  ss << "    Trigger time = " << trigger_resolution_ns_ << "*trigger (ns)\n";
  ss << "    Maximum chip time in window = " << max_chip_time_in_window_ns_ << " (ns)\n";
  ss << "    Target resolution = " << target_resolution_ns_ << "  (ns)\n";
  return ss.str();
}

