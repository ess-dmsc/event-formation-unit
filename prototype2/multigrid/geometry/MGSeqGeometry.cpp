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

void MGSeqGeometry::max_wire(uint16_t g) {
  max_wire_ = g;
}

void MGSeqGeometry::max_channel(uint16_t g) {
  max_channel_ = g;
}

void MGSeqGeometry::max_z(uint16_t w) {
  max_z_ = w;
}

void MGSeqGeometry::flipped_x(bool f) {
  flipped_x_ = f;
}

void MGSeqGeometry::flipped_z(bool f) {
  flipped_z_ = f;
}

bool MGSeqGeometry::swap_wires() const {
  return swap_wires_;
}

bool MGSeqGeometry::swap_grids() const {
  return swap_grids_;
}

bool MGSeqGeometry::flipped_x() const {
  return flipped_x_;
}

bool MGSeqGeometry::flipped_z() const {
  return flipped_z_;
}

uint32_t MGSeqGeometry::max_x() const {
  return max_wire_ / max_z_;
}

uint32_t MGSeqGeometry::max_y() const {
  return max_grid();
}

uint16_t MGSeqGeometry::max_z() const {
  return max_z_;
}

uint16_t MGSeqGeometry::max_channel() const {
  return max_channel_;
}

uint16_t MGSeqGeometry::max_wire() const {
  return max_wire_;
}

uint16_t MGSeqGeometry::max_grid() const {
  return max_channel_ - max_wire_;
}

bool MGSeqGeometry::isWire(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  return (channel < max_wire_);
}

bool MGSeqGeometry::isGrid(uint8_t VMM, uint16_t channel) const {
  (void) VMM;
  return (channel >= max_wire_) && (channel < max_channel_);
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
  return channel - max_wire_;
}

uint32_t MGSeqGeometry::x_from_wire(uint16_t w) const {
  if (flipped_x_) {
    return (max_wire_ / max_z_) - uint16_t(1) - w / max_z_;
  } else {
    return w / max_z_;
  }
}

uint32_t MGSeqGeometry::y_from_grid(uint16_t g) const {
  return g;
}

uint32_t MGSeqGeometry::z_from_wire(uint16_t w) const {
  if (flipped_z_) {
    return (max_z_ - uint16_t(1)) - w % max_z_;
  } else {
    return w % max_z_;
  }
}

std::string MGSeqGeometry::debug(std::string prefix) const {
  std::stringstream ss;

  ss << prefix << "wires=chan[0," << (max_wire_ - 1) << "] ";
  if (swap_wires_)
    ss << "(swapped)";
  ss << "\n";

  ss << prefix << "grids=chan[" << max_wire_ << "," << (max_channel_ - 1) << "] ";
  if (swap_grids_)
    ss << "(swapped)";
  ss << "\n";

  ss << prefix << "size [" << max_x() << "," << max_y() << "," << max_z() << "]\n";
  if (flipped_x_)
    ss << prefix << "(flipped in X)\n";
  if (flipped_z_)
    ss << prefix << "(flipped in Z)\n";

  ss << ModuleGeometry::debug(prefix);

  return ss.str();
}

void from_json(const nlohmann::json &j, MGSeqGeometry &g) {
  g.max_channel(j["max_channel"]);
  g.max_wire(j["max_wire"]);
  g.max_z(j["max_z"]);

  if (j.count("swap_wires"))
    g.swap_wires(j["swap_wires"]);
  if (j.count("swap_grids"))
    g.swap_grids(j["swap_grids"]);
  if (j.count("flipped_x"))
    g.flipped_x(j["flipped_x"]);
  if (j.count("flipped_z"))
    g.flipped_z(j["flipped_z"]);

  if (j.count("wire_filters")) {
    auto wf = j["wire_filters"];
    if (wf.count("blanket"))
      g.set_wire_filters(wf["blanket"]);
    if (wf.count("exceptions")) {
      auto wfe = wf["exceptions"];
      for (unsigned int j = 0; j < wfe.size(); j++) {
        g.override_wire_filter(wfe[j]["idx"], wfe[j]);
      }
    }
  }

  if (j.count("grid_filters")) {
    auto gf = j["grid_filters"];
    if (gf.count("blanket"))
      g.set_grid_filters(gf["blanket"]);
    if (gf.count("exceptions")) {
      auto gfe = gf["exceptions"];
      for (unsigned int j = 0; j < gfe.size(); j++) {
        g.override_grid_filter(gfe[j]["idx"], gfe[j]);
      }
    }
  }

}

}

