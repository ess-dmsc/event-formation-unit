/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>

// TODO: initialize values to 0?
struct Readout
{
  uint8_t fec;
  uint8_t chip_id;
  uint32_t bonus_timestamp;
  uint32_t srs_timestamp;
  uint16_t channel;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  float ChipTimeNs;
  bool over_threshold;

  bool operator==(const Readout& other) const
  {
    return (
        (fec == other.fec) &&
        (chip_id == other.chip_id) &&
        (bonus_timestamp == other.bonus_timestamp) &&
        (srs_timestamp == other.srs_timestamp) &&
        (channel == other.channel) &&
        (bcid == other.bcid) &&
        (tdc == other.tdc) && (adc == other.adc) &&
        (ChipTimeNs == other.ChipTimeNs) &&
        (over_threshold == other.over_threshold)
    );
  }
};
