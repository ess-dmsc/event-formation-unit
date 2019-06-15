/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/Filter.h>

namespace Multigrid {

class ModuleChannelMappings {
public:
  FilterSet wire_filters;
  FilterSet grid_filters;

  virtual ~ModuleChannelMappings() = default;

  virtual uint16_t max_channel() const = 0;

  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns wire */
  virtual uint16_t wire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns grid */
  virtual uint16_t grid(uint8_t VMM, uint16_t channel) const = 0;

  virtual std::string debug(std::string prefix) const;
};

void from_json(const nlohmann::json &j, ModuleChannelMappings &g);

}

