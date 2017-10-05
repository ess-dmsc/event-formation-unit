/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX plane ID & strip mappings
 */

#pragma once

#include <initializer_list>
#include <cinttypes>
#include <vector>
#include <string>

#define NMX_MAX_CHIPS 16
#define NMX_CHIP_CHANNELS 64
#define NMX_INVALID_GEOM_ID ((uint16_t)(int16_t)(-1))
#define NMX_INVALID_PLANE_ID ((uint8_t)(int8_t)(-1))

class SRSMappings {
public:
  /** @brief define mappings for sequence of chips in one plane
   * @param planeID ID of plane (edge of panel) being defined
   * @param chips list of (FEC, VMM) pairs in the order of increasing strip
   * number
   */
  void define_plane(uint8_t planeID,
                    std::initializer_list<std::pair<uint16_t, uint16_t>> chips);

  /** @brief define channel mappings for one chip
   * @param fecID ID of FEC
   * @param vmmID ID of chip
   * @param planeID ID of plane (edge of panel)
   * @param strip_offset starting strip for this chip's channels
   */
  void set_mapping(uint16_t fecID, uint16_t vmmID, uint8_t planeID,
                   uint16_t strip_offset);

  /** @brief get strip number
   * @param fecID ID of FEC
   * @param vmmID ID of chip
   * @param channelID channel number
   */
  uint16_t get_strip(uint16_t fecID, uint16_t vmmID, uint32_t channelID) const;

  /** @brief get plane ID
   * @param fecID ID of FEC
   * @param vmmID ID of chip
   */
  uint8_t get_plane(uint16_t fecID, uint16_t vmmID) const;

  // @brief prints out configuration
  std::string debug() const;

private:
  std::vector<std::vector<uint16_t>> offsets_; // strip number mappings
  std::vector<std::vector<uint8_t>> planes_;  // planeID mappings
};
