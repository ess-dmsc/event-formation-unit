/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/SumoMappings.h>
#include <fmt/format.h>
#include <jalousie/rapidcsv.h>

namespace Jalousie {

bool SumoCoordinates::is_valid() const {
  return ((wire_layer != kInvalidCoord) &&
      (wire != kInvalidCoord) &&
      (strip != kInvalidCoord));
}

std::string SumoCoordinates::debug() const {
  if (!is_valid())
    return "INV";
  return fmt::format("(wl{},w{},s{})", wire_layer, wire, strip);
}

SumoCoordinates min(const SumoCoordinates &a,
                                  const SumoCoordinates &b) {
  SumoCoordinates ret;
  ret.wire_layer = std::min(a.wire_layer, b.wire_layer);
  ret.wire = std::min(a.wire, b.wire);
  ret.strip = std::min(a.strip, b.strip);
  return ret;
}

SumoCoordinates max(const SumoCoordinates &a,
                                  const SumoCoordinates &b) {
  SumoCoordinates ret;
  ret.wire_layer = std::max(a.wire_layer, b.wire_layer);
  ret.wire = std::max(a.wire, b.wire);
  ret.strip = std::max(a.strip, b.strip);
  return ret;
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
  if (!coord.is_valid())
    return;

  /// Initialize or update max coordinate
  if (table_.empty()) {
    min_ = coord;
    max_ = coord;
  }
  min_ = Jalousie::min(min_, coord);
  max_ = Jalousie::max(max_, coord);

  /// Resize table on demand
  if (table_.size() <= anode)
    table_.resize(anode + 1);
  auto &anode_line = table_[anode];
  if (anode_line.size() <= cathode)
    anode_line.resize(cathode + 1);

  /// Finally, assign
  anode_line[cathode] = coord;
}

SumoCoordinates SumoMappings::map(uint8_t anode, uint8_t cathode) const {
  if (table_.size() <= anode)
    return {};
  const auto &anode_line = table_[anode];
  if (anode_line.size() <= cathode)
    return {};
  return anode_line[cathode];
}

SumoCoordinates SumoMappings::min() const {
  return min_;
}

SumoCoordinates SumoMappings::max() const {
  return max_;
}

std::string SumoMappings::debug(bool depict_domain) const {
  std::string ret;
  ret += " --> [" + min_.debug() + " - " + max_.debug();
  if (!depict_domain)
    return ret;
  ret += "\n";
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