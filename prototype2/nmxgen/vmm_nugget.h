/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <inttypes.h>

struct vmm_nugget 
{
  uint64_t time;
  uint8_t plane_id;
  uint8_t strip;
  uint16_t adc;
};
