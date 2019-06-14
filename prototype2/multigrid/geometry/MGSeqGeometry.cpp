/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/MGSeqGeometry.h>

#include <sstream>
#include <limits>

namespace Multigrid {

void MGSeqGeometry::swap(uint16_t &channel) {
  if (channel % 2 == 0) {
    channel += 1;
  } else {
    channel -= 1;
  }
}

void MGSeqGeometry::swap_wires(bool s) {
  swap_wires_ = s;
}

void MGSeqGeometry::swap_grids(bool s) {
  swap_grids_ = s;
}

void MGSeqGeometry::max_channel(uint16_t g) {
  max_channel_ = g;
  max_grid(g - max_wire());
}

bool MGSeqGeometry::swap_wires() const {
  return swap_wires_;
}

bool MGSeqGeometry::swap_grids() const {
  return swap_grids_;
}

uint16_t MGSeqGeometry::max_channel() const {
  return max_channel_;
}

bool MGSeqGeometry::isWire(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  return (channel < max_wire());
}

bool MGSeqGeometry::isGrid(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  return (channel >= max_wire()) && (channel < max_channel_);
}

uint16_t MGSeqGeometry::wire(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  if (swap_wires_) {
    swap(channel);
  }
  return channel;
}

uint16_t MGSeqGeometry::grid(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  if (swap_grids_) {
    swap(channel);
  }
  return channel - max_wire();
}

std::string MGSeqGeometry::debug(std::string prefix) const {
  std::stringstream ss;

  ss << prefix << "wires=chan[0," << (max_wire() - 1) << "] ";
  if (swap_wires_)
    ss << "(swapped wires)";
  ss << "\n";

  ss << prefix << "grids=chan[" << max_wire() << "," << (max_channel_ - 1) << "] ";
  if (swap_grids_)
    ss << "(swapped grids)";
  ss << "\n";

  ss << prefix << "size [" << max_x() << "," << max_y() << "," << max_z() << "]\n";

  ss << ModuleGeometry::debug(prefix);

  return ss.str();
}

void from_json(const nlohmann::json &j, MGSeqGeometry &g) {

  g.max_wire(j["max_wire"]);
  g.max_channel(j["max_channel"]);
  g.max_z(j["max_z"]);

  if (j.count("swap_wires"))
    g.swap_wires(j["swap_wires"]);
  if (j.count("swap_grids"))
    g.swap_grids(j["swap_grids"]);

  // same as for ModuleGeometry, could refactor?

  if (j.count("flipped_x"))
    g.flipped_x(j["flipped_x"]);
  if (j.count("flipped_z"))
    g.flipped_z(j["flipped_z"]);

  if (j.count("wire_filters")) {
    g.wire_filters = j["wire_filters"];
  }

  if (j.count("grid_filters")) {
    g.grid_filters = j["grid_filters"];
  }
}

}

