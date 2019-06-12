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

  ret += prefix + fmt::format("  size [{},{},{}]\n",
                     max_x(), max_y(), max_z());

  if (flipped_x_)
    ret += prefix + "  (flipped in X)\n";
  if (flipped_z_)
    ret += prefix + "  (flipped in Z)\n";

  return ret;
}

uint16_t ModuleGeometry::rescale_wire(uint16_t wire, uint16_t adc) const {
  if (wire >= wire_filters_.size())
    return adc;
  return wire_filters_[wire].rescale(adc);
}

uint16_t ModuleGeometry::rescale_grid(uint16_t grid, uint16_t adc) const {
  if (grid >= grid_filters_.size())
    return adc;
  return grid_filters_[grid].rescale(adc);
}

bool ModuleGeometry::valid_wire(uint16_t wire, uint16_t adc) const {
  if (wire >= wire_filters_.size())
    return true;
  return wire_filters_[wire].valid(adc);
}

bool ModuleGeometry::valid_grid(uint16_t grid, uint16_t adc) const {
  if (grid >= grid_filters_.size())
    return true;
  return grid_filters_[grid].valid(adc);
}

void ModuleGeometry::set_wire_filters(Filter mgf) {
  wire_filters_.resize(this->max_wire() + 1u);
  for (auto &f : wire_filters_)
    f = mgf;
}

void ModuleGeometry::set_grid_filters(Filter mgf) {
  grid_filters_.resize(this->max_grid() + 1u);
  for (auto &f : grid_filters_)
    f = mgf;
}

void ModuleGeometry::override_wire_filter(uint16_t n, Filter mgf) {
  if (wire_filters_.size() <= n)
    wire_filters_.resize(n + 1);
  wire_filters_[n] = mgf;
}

void ModuleGeometry::override_grid_filter(uint16_t n, Filter mgf) {
  if (grid_filters_.size() <= n)
    grid_filters_.resize(n + 1);
  grid_filters_[n] = mgf;
}

std::string ModuleGeometry::debug(std::string prefix) const {
  std::stringstream ss;

  ss << ModuleLogicalGeometry::debug(prefix);

  std::stringstream wfilters;
  bool validwf{false};
  for (size_t i = 0; i < wire_filters_.size(); i++) {
    const auto &f = wire_filters_[i];
    if (!f.trivial()) {
      wfilters << prefix << "  [" << i << "]  " << f.debug() << "\n";
      validwf = true;
    }
  }
  if (validwf) {
    ss << prefix << "Wire filters:\n" << wfilters.str();
  }

  std::stringstream gfilters;
  bool validgf{false};
  for (size_t i = 0; i < grid_filters_.size(); i++) {
    const auto &f = grid_filters_[i];
    if (!f.trivial()) {
      gfilters << prefix << "  [" << i << "]  " << f.debug() << "\n";
      validgf = true;
    }
  }
  if (validgf) {
    ss << prefix << "Grid filters:\n" << gfilters.str();
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, ModuleGeometry &g) {

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
      for (auto k = 0u; k < wfe.size(); k++) {
        g.override_wire_filter(wfe[k]["idx"], wfe[k]);
      }
    }
  }

  if (j.count("grid_filters")) {
    auto gf = j["grid_filters"];
    if (gf.count("blanket"))
      g.set_grid_filters(gf["blanket"]);
    if (gf.count("exceptions")) {
      auto gfe = gf["exceptions"];
      for (auto k = 0u; k < gfe.size(); k++) {
        g.override_grid_filter(gfe[k]["idx"], gfe[k]);
      }
    }
  }

}

}

