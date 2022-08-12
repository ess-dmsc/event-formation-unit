/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/ChannelMappings.h>

namespace Multigrid {

class MGSeqMappings : public ChannelMappings {
public:
  // Configuration
  void swap_wires(bool s);
  void swap_grids(bool s);
  void max_channel(uint16_t g);
  bool swap_wires() const;
  bool swap_grids() const;
  void max_wire(uint16_t w);

  // Implementation

  uint16_t max_channel() const override;

  /** @brief identifies which channels are wires, from drawing by Anton */
  bool isWire(uint16_t channel) const override;

  /** @brief identifies which channels are grids, from drawing by Anton */
  bool isGrid(uint16_t channel) const override;

  /** @returns wire */
  uint16_t wire(uint16_t channel) const override;

  /** @returns grid */
  uint16_t grid(uint16_t channel) const override;

  /** @returns maximum wire */
  uint16_t max_wire() const override;

  /** @returns maximum grid */
  uint16_t max_grid() const override;

  std::string debug(std::string prefix) const override;

protected:
  static void swap(uint16_t &channel);

private:
  uint16_t max_channel_{120};
  uint16_t max_wire_{80};

  bool swap_wires_{false};
  bool swap_grids_{false};
};

void from_json(const nlohmann::json &j, MGSeqMappings &g);

}

