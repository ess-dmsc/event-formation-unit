/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/ModuleGeometry.h>

#include <fmt/format.h>
#include <sstream>

namespace Multigrid {

void ModuleLogicalGeometry::max_grid(uint16_t g) {
  max_grid_ = g;
}

void ModuleLogicalGeometry::max_wire(uint16_t w) {
  max_wire_ = w;
}

void ModuleLogicalGeometry::max_z(uint16_t w) {
  max_z_ = w;
}

void ModuleLogicalGeometry::flipped_x(bool f) {
  flipped_x_ = f;
}

void ModuleLogicalGeometry::flipped_z(bool f) {
  flipped_z_ = f;
}

bool ModuleLogicalGeometry::flipped_x() const {
  return flipped_x_;
}

bool ModuleLogicalGeometry::flipped_z() const {
  return flipped_z_;
}

uint32_t ModuleLogicalGeometry::max_x() const {
  return max_wire() / max_z();
}

uint32_t ModuleLogicalGeometry::max_y() const {
  return max_grid();
}

uint16_t ModuleLogicalGeometry::max_z() const {
  return max_z_;
}

uint16_t ModuleLogicalGeometry::max_wire() const {
  return max_wire_;
}

uint16_t ModuleLogicalGeometry::max_grid() const {
  return max_grid_;
}

uint32_t ModuleLogicalGeometry::x_from_wire(uint16_t w) const {
  uint32_t ret = w / max_z();
  return flipped_x() ? (max_x() - 1u - ret) : ret;
}

uint32_t ModuleLogicalGeometry::y_from_grid(uint16_t g) const {
  return g;
}

uint32_t ModuleLogicalGeometry::z_from_wire(uint16_t w) const {
  uint32_t ret = w % max_z();
  return flipped_z() ? (max_z() - 1u - ret) : ret;
}

std::string ModuleLogicalGeometry::debug(std::string prefix) const {
  std::string ret;

  ret += prefix + "ModuleLogicalGeometry\n";
  ret += prefix + fmt::format("  wires: {},  grids: {}\n",
                              max_wire(), max_grid());
  ret += prefix + fmt::format("  size [{},{},{}]\n",
                              max_x(), max_y(), max_z());

  if (flipped_x_)
    ret += prefix + "  (flipped in X)\n";
  if (flipped_z_)
    ret += prefix + "  (flipped in Z)\n";

  return ret;
}

void from_json(const nlohmann::json &j, ModuleLogicalGeometry &g) {
  if (j.count("max_grid"))
    g.max_grid(j["max_grid"]);
  if (j.count("max_wire"))
    g.max_wire(j["max_wire"]);
  if (j.count("max_z"))
    g.max_z(j["max_z"]);

  if (j.count("flipped_x"))
    g.flipped_x(j["flipped_x"]);
  if (j.count("flipped_z"))
    g.flipped_z(j["flipped_z"]);
}

}

