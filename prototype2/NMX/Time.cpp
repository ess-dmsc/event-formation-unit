/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Time.h>

double Time::timestamp_ns(uint32_t trigger, uint16_t bc, uint16_t tdc) const
{
    //BC time: bcid value * 1/(clock frequency)
  double bcTime = double(bc) / bc_clock_;
  //TDC time: tacSlope * tdc value (8 bit) * ramp length
  double tdcTime = double(tdc) * tac_slope_ / 256.0;
  //Chip time: bcid plus tdc value
  double chip_time = bcTime * 1000 + tdcTime;

  double trigger_timestamp_ns = trigger * trigger_resolution_;
  return trigger_timestamp_ns + chip_time;
}

uint16_t Time::timestamp(uint32_t trigger, uint16_t bc, uint16_t tdc) const
{
  return timestamp_ns(trigger, bc, tdc) * target_resolution_ns_;
}
