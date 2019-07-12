/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/SumoMappings.h>
#include <fmt/format.h>
#include <jalousie/rapidcsv.h>

namespace Jalousie {

bool SumoMappings::SumoCoordinates::is_valid() const {
  return ((wire_layer != kInvalidCoord) &&
      (wire != kInvalidCoord) &&
      (strip != kInvalidCoord));
}

std::string SumoMappings::SumoCoordinates::debug() const {
  if (!is_valid())
    return "INV";
  return fmt::format("(wl{},w{},s{})", wire_layer, wire, strip);
}

SumoMappings::SumoMappings(std::string file_name, uint8_t sumo_id) {
  rapidcsv::Document doc(file_name,
                         rapidcsv::LabelParams(0, -1),
                         rapidcsv::SeparatorParams(';'));
  std::vector<int> line;
  SumoCoordinates coord;
  int sumo_type;
  int anode;
  int cathode;
  for (size_t i = 0; i < doc.GetRowCount(); ++i) {
    line = doc.GetRow<int>(i);

    sumo_type = line[0];
    if (sumo_type != sumo_id)
      continue;

    coord.wire_layer = line[1];
    coord.wire = line[2];
    coord.strip = line[3];
    anode = line[4];
    cathode = line[5];

    add_mapping(anode, cathode, coord);
  }
}

void SumoMappings::add_mapping(uint8_t anode, uint8_t cathode, SumoCoordinates coord) {
  if (table_.size() <= anode)
    table_.resize(anode + 1);
  auto &anode_line = table_[anode];
  if (anode_line.size() <= cathode)
    anode_line.resize(cathode + 1, {SumoCoordinates::kInvalidCoord,
                                    SumoCoordinates::kInvalidCoord,
                                    SumoCoordinates::kInvalidCoord});
  anode_line[cathode] = coord;
}

SumoMappings::SumoCoordinates SumoMappings::map(uint8_t anode, uint8_t cathode) const {
  if (table_.size() <= anode)
    return {SumoCoordinates::kInvalidCoord,
            SumoCoordinates::kInvalidCoord,
            SumoCoordinates::kInvalidCoord};
  const auto &anode_line = table_[anode];
  if (anode_line.size() <= cathode)
    return {SumoCoordinates::kInvalidCoord,
            SumoCoordinates::kInvalidCoord,
            SumoCoordinates::kInvalidCoord};
  return anode_line[cathode];
}

std::string SumoMappings::debug() const {
  std::string ret;
  for (size_t anode = 0; anode < table_.size(); ++anode) {
    const auto &anode_line = table_[anode];
    ret += fmt::format("{:<5}", anode);
    for (size_t cathode = 0; cathode < anode_line.size(); ++cathode) {
      ret += anode_line[cathode].is_valid() ? "@" : "-";
    }
    ret += "\n";
  }
  return ret;
}

}