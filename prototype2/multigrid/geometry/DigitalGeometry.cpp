/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Sequoia
 * detector demonstrators although not all channels may be in use
 */

#include <multigrid/geometry/DigitalGeometry.h>
#include <fmt/format.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

void DetectorGeometry::add_bus(ModuleGeometry geom) {
  buses.push_back(geom);
}

uint32_t DetectorGeometry::x_from_wire(size_t module, uint16_t w) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for x_from_wire");
  }
  const auto &b = buses[module];
  return b.x_from_wire(w);
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::y_from_grid(size_t module, uint16_t g) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for y_from_grid");
  }
  const auto &b = buses[module];
  return b.y_from_grid(g);
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::z_from_wire(size_t module, uint16_t w) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for z_from_wire");
  }
  const auto &b = buses[module];
  return b.z_from_wire(w);
}

/** @brief return the x coordinate of the detector */
uint32_t DetectorGeometry::max_x() const {
  uint32_t ret{0};
  for (const auto& b : buses)
    ret = std::max(ret, b.x_offset + b.x_range());
  return ret;
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::max_y() const {
  uint32_t ret{0};
  for (const auto& b : buses)
    ret = std::max(ret, b.y_offset + b.y_range());
  return ret;
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::max_z() const {
  uint32_t ret{0};
  for (const auto& b : buses)
    ret = std::max(ret, b.z_offset + b.z_range());
  return ret;
}

std::string DetectorGeometry::debug(std::string prefix) const {
  std::stringstream ss;
  ss << prefix + fmt::format("  Extents [{},{},{}]\n",
                              max_x(), max_y(), max_z());
  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "\n" << buses[i].debug(prefix + " ");
  }
  return ss.str();
}

void from_json(const nlohmann::json &j, DetectorGeometry &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    g.add_bus(j[i]);
  }
}

}
