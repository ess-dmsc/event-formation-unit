/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Sequoia
 * detector demonstrators although not all channels may be in use
 */

#pragma once

#include <multigrid/geometry/ChannelMappings.h>

namespace Multigrid {

// \todo make this private within DetectorMapping scope
struct ModuleMapping {
  /// For actual plane mapping
  uint8_t plane_offset{0};

  /// For absolute wire/grid numbers
  uint16_t wire_offset{0};
  uint16_t grid_offset{0};

  std::shared_ptr<ChannelMappings> channel_mappings;

  std::string debug(std::string prefix = "") const;
};

class DetectorMapping {
public:
  void add_bus(std::shared_ptr<ChannelMappings>);

  // \todo document this
  bool map(Hit &hit, uint8_t bus, uint16_t channel, uint16_t adc) const;

  // \todo document this
  // plane becomes 0 or 1, regardless of module
  // coord becomes total grid/wire number
  Hit absolutify(const Hit &original) const;

  uint16_t max_wire() const;

  uint16_t max_grid() const;

  std::string debug(std::string prefix = "") const;

//private:
  std::vector<ModuleMapping> buses;
};

void from_json(const nlohmann::json &j, DetectorMapping &g);

}
