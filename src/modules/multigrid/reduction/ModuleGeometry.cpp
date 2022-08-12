/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/reduction/ModuleGeometry.h>

#include <fmt/format.h>
#include <sstream>

namespace Multigrid {

void ModuleGeometry::num_grids(uint16_t g) {
  grids_ = g;
}

void ModuleGeometry::num_wires(uint16_t w) {
  wires_ = w;
  x_range_ = wires_ / z_range_;
}

void ModuleGeometry::z_range(uint32_t z) {
  z_range_ = z;
  x_range_ = wires_ / z_range_;
}

void ModuleGeometry::flipped_x(bool f) {
  flipped_x_ = f;
}

void ModuleGeometry::flipped_z(bool f) {
  flipped_z_ = f;
}

bool ModuleGeometry::flipped_x() const {
  return flipped_x_;
}

bool ModuleGeometry::flipped_z() const {
  return flipped_z_;
}

// \todo cache this result
uint32_t ModuleGeometry::x_range() const {
  return x_range_;
}

uint32_t ModuleGeometry::y_range() const {
  return num_grids();
}

uint32_t ModuleGeometry::z_range() const {
  return z_range_;
}

uint16_t ModuleGeometry::num_wires() const {
  return wires_;
}

uint16_t ModuleGeometry::num_grids() const {
  return grids_;
}

inline uint32_t flip(uint32_t val, uint32_t max) {
  return max - val - 1u;
}

uint32_t ModuleGeometry::x_from_wire(uint16_t w) const {
  uint32_t ret = w / z_range_;
  return x_offset + (flipped_x() ? flip(ret, x_range_) : ret);
}

uint32_t ModuleGeometry::y_from_grid(uint16_t g) const {
  return y_offset + g;
}

uint32_t ModuleGeometry::z_from_wire(uint16_t w) const {
  uint32_t ret = w % z_range_;
  return z_offset + (flipped_z() ? flip(ret, z_range_) : ret);
}

std::string ModuleGeometry::debug(std::string prefix) const {
  std::string ret;

  ret += prefix + fmt::format("  wires: {},  grids: {}\n",
                              num_wires(), num_grids());

  ret += prefix + fmt::format("  x[{},{}] y[{},{}] z[{},{}]\n",
                              x_offset, x_offset + x_range() - 1u,
                              y_offset, y_offset + y_range() - 1u,
                              z_offset, z_offset + z_range() - 1u
                              );

  if (flipped_x_)
    ret += prefix + "  (flipped in X)\n";
  if (flipped_z_)
    ret += prefix + "  (flipped in Z)\n";

  return ret;
}

void from_json(const nlohmann::json &j, ModuleGeometry &g) {
  if (j.count("num_grids"))
    g.num_grids(j["num_grids"]);
  if (j.count("num_wires"))
    g.num_wires(j["num_wires"]);
  if (j.count("z_range"))
    g.z_range(j["z_range"]);

  if (j.count("flipped_x"))
    g.flipped_x(j["flipped_x"]);
  if (j.count("flipped_z"))
    g.flipped_z(j["flipped_z"]);

  if (j.count("x_offset"))
    g.x_offset = j["x_offset"];
  if (j.count("y_offset"))
    g.x_offset = j["y_offset"];
  if (j.count("z_offset"))
    g.x_offset = j["z_offset"];
}

}

