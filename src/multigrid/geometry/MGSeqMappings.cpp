/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/MGSeqMappings.h>

#include <sstream>

namespace Multigrid {

void MGSeqMappings::swap(uint16_t &channel) {
  if (channel % 2 == 0) {
    channel += 1;
  } else {
    channel -= 1;
  }
}

void MGSeqMappings::max_wire(uint16_t w) {
  max_wire_ = w;
}

uint16_t MGSeqMappings::max_wire() const {
  return max_wire_;
}

uint16_t MGSeqMappings::max_grid() const {
  return max_channel_ - max_wire_;
}

void MGSeqMappings::swap_wires(bool s) {
  swap_wires_ = s;
}

void MGSeqMappings::swap_grids(bool s) {
  swap_grids_ = s;
}

void MGSeqMappings::max_channel(uint16_t g) {
  max_channel_ = g;
}

bool MGSeqMappings::swap_wires() const {
  return swap_wires_;
}

bool MGSeqMappings::swap_grids() const {
  return swap_grids_;
}

uint16_t MGSeqMappings::max_channel() const {
  return max_channel_;
}

bool MGSeqMappings::isWire(uint16_t channel) const {
  return (channel < max_wire());
}

bool MGSeqMappings::isGrid(uint16_t channel) const {
  return (channel >= max_wire()) && (channel < max_channel_);
}

uint16_t MGSeqMappings::wire(uint16_t channel) const {
  if (swap_wires_) {
    swap(channel);
  }
  return channel;
}

uint16_t MGSeqMappings::grid(uint16_t channel) const {
  if (swap_grids_) {
    swap(channel);
  }
  return channel - max_wire();
}

std::string MGSeqMappings::debug(std::string prefix) const {
  std::stringstream ss;

  ss << "[MGSeqMappings]\n";

  ss << prefix << "wires=chan[0," << (max_wire() - 1) << "]";
  if (swap_wires_)
    ss << " (swapped)";
  ss << "\n";

  ss << prefix << "grids=chan[" << max_wire() << "," << (max_channel_ - 1) << "]";
  if (swap_grids_)
    ss << " (swapped)";
  ss << "\n";

  ss << ChannelMappings::debug(prefix);

  return ss.str();
}

void from_json(const nlohmann::json &j, MGSeqMappings &g) {

  g.max_channel(j["max_channel"]);
  g.max_wire(j["max_wire"]);

  if (j.count("swap_wires"))
    g.swap_wires(j["swap_wires"]);
  if (j.count("swap_grids"))
    g.swap_grids(j["swap_grids"]);

  /// same as for ChannelMappings, could refactor?

  if (j.count("wire_filters")) {
    g.wire_filters = j["wire_filters"];
  }

  if (j.count("grid_filters")) {
    g.grid_filters = j["grid_filters"];
  }
}

}

