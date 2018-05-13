#pragma once

#include <cinttypes>

struct Readout
{
  uint8_t fec;
  uint8_t chip_id;
  uint32_t frame_counter;
  uint32_t srs_timestamp;
  uint16_t channel;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;

  bool operator==(const Readout& other) const
  {
    return (
        (fec == other.fec) &&
        (chip_id == other.chip_id) &&
        (frame_counter == other.frame_counter) &&
        (srs_timestamp == other.srs_timestamp) &&
        (channel == other.channel) &&
        (bcid == other.bcid) &&
        (tdc == other.tdc) && (adc == other.adc) &&
        (over_threshold == other.over_threshold)
    );
  }
};
