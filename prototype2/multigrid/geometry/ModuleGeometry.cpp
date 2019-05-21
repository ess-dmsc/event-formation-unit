/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/ModuleGeometry.h>

#include <sstream>
#include <limits>

namespace Multigrid {

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
  wire_filters_.resize(max_wire());
  for (auto &f : wire_filters_)
    f = mgf;
}

void ModuleGeometry::set_grid_filters(Filter mgf) {
  grid_filters_.resize(max_grid());
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

