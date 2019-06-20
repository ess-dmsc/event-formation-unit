/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <common/clustering/Hit.h>
#include <multigrid/geometry/Filter.h>

namespace Multigrid {

class ChannelMappings {
public:
  static constexpr uint8_t wire_plane {0};
  static constexpr uint8_t grid_plane {1};

  FilterSet wire_filters;
  FilterSet grid_filters;

  virtual ~ChannelMappings() = default;

  virtual uint16_t max_channel() const = 0;

  // \todo document this
  bool map(Hit& hit, uint16_t channel, uint16_t adc) const;

  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint16_t channel) const = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint16_t channel) const = 0;

  /** @brief returns wire */
  virtual uint16_t wire(uint16_t channel) const = 0;

  /** @brief returns grid */
  virtual uint16_t grid(uint16_t channel) const = 0;

  virtual std::string debug(std::string prefix) const;
};

void from_json(const nlohmann::json &j, ChannelMappings &g);

}

