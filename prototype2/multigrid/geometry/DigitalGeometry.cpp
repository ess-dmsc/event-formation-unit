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

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

std::string ModuleGeometry::debug(std::string prefix) const {
  std::stringstream ss;
  ss << "x+" << x_offset << "  "
     << "y+" << y_offset << "  "
     << "z+" << z_offset << "  "
     << "\n";
  ss << logical_geometry.debug(prefix + "  ");
  return ss.str();
}

void from_json(const nlohmann::json &j, ModuleGeometry &g) {
  // \todo set max grid and wire
  g.logical_geometry = j["geometry"];
  //.max_grid(geom->max_channel() - geom->max_wire());
}

void DetectorGeometry::add_bus(ModuleLogicalGeometry geom) {
  ModuleGeometry mg;
  mg.logical_geometry = geom;
  for (const auto &d : buses) {
    mg.x_offset += d.logical_geometry.max_x();
    //g.y_offset += d.y_offset + d.channel_mappings.max_y();
    //g.z_offset += d.z_offset + d.channel_mappings.max_z();
  }
  buses.push_back(mg);
}

uint32_t DetectorGeometry::x_from_wire(size_t module, uint16_t w) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for x_from_wire");
  }
  const auto &b = buses[module];
  return b.x_offset + b.logical_geometry.x_from_wire(w);
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::y_from_grid(size_t module, uint16_t g) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for y_from_grid");
  }
  const auto &b = buses[module];
  return b.y_offset + b.logical_geometry.y_from_grid(g);
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::z_from_wire(size_t module, uint16_t w) const {
  if (module >= buses.size()) {
    throw std::runtime_error("No bus definition for z_from_wire");
  }
  const auto &b = buses[module];
  return b.z_offset + b.logical_geometry.z_from_wire(w);
}

/** @brief return the x coordinate of the detector */
uint32_t DetectorGeometry::max_x() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.x_offset + b.logical_geometry.max_x();
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::max_y() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.y_offset + b.logical_geometry.max_y();
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::max_z() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.z_offset + b.logical_geometry.max_z();
}

std::string DetectorGeometry::debug(std::string prefix) const {
  std::stringstream ss;
  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "   " << buses[i].debug(prefix);
  }
  return ss.str();
}

void from_json(const nlohmann::json &j, DetectorGeometry &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    g.add_bus(j[i]);
  }
}

/*
void from_json(const nlohmann::json &j, BusDefinitionStruct &g) {
 auto geom = std::make_shared<MGSeqMappings>();
 (*geom) = j;
 g.channel_mappings = geom;
 g.logical_geometry.max_grid(geom->max_channel() - geom->max_wire());
}

void DigitalGeometry::add_bus(BusDefinitionStruct geom) {
 for (const auto &d : buses2) {
   geom.wire_offset += d.logical_geometry.max_wire();
   geom.grid_offset += d.logical_geometry.max_grid();
   geom.x_offset += d.logical_geometry.max_x();
   //g.y_offset += d.y_offset + d.channel_mappings.max_y();
   //g.z_offset += d.z_offset + d.channel_mappings.max_z();
 }
 buses2.push_back(geom);

 ModuleMapping mm;
 mm.wire_offset = geom.wire_offset;
 mm.grid_offset = geom.grid_offset;
 mm.wires = geom.logical_geometry.max_wire();
 mm.grids = geom.logical_geometry.max_grid();
 mm.channel_mappings = geom.channel_mappings;
 mappings.push_back(mm);

 ModuleGeometry mg;
 mg.x_offset = geom.x_offset;
 mg.logical_geometry = geom.logical_geometry;
 geometries.push_back(mg);
}
*/

}
