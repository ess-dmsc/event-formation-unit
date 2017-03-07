/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX plane ID & strip mappings
 */

#pragma once

#include <vector>
#include <initializer_list>
#include <inttypes.h>

#define VMM_TOTAL_CHANNELS 64
#define VMM_INVALID ((uint16_t)(int16_t)(-1))

class Geometry {
public:

  void define_plane(uint16_t planeID,
                    std::initializer_list<std::pair<uint16_t, uint16_t>> chips);

  void set_mapping(uint16_t fecID, uint16_t vmmID,
                   uint16_t planeID, uint16_t strip_offset);

  uint16_t get_strip_ID(uint16_t fecID, uint16_t vmmID, uint32_t channelID) const;
  uint16_t get_plane_ID(uint16_t fecID, uint16_t vmmID) const;

private:
  std::vector<std::vector<uint16_t>> offsets_;
  std::vector<std::vector<uint16_t>> planes_;
};
