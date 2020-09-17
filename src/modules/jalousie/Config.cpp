// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie detector configuration implementation
///
//===----------------------------------------------------------------------===//

#include <jalousie/Config.h>
#include <sstream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
#include <common/Log.h>
#include <common/Trace.h>

namespace Jalousie {

static constexpr size_t invalid_board_mapping {std::numeric_limits<size_t>::max()};

Config::Config(std::string jsonfile) {
  nlohmann::json root;
  try {
    std::ifstream t(jsonfile);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    if (!t.good()) {
      XTRACE(INIT, ERR, "Invalid Json file: %s", jsonfile.c_str());
      throw std::runtime_error("Jalousie config file error - requested file unavailable.");
    }

    root = nlohmann::json::parse(str);
  }
  catch (...) {
    LOG(INIT, Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  auto root_dir = boost::filesystem::path(jsonfile).parent_path();

  std::string SUMO_mappings_file = root["SUMO_mappings_file"];
  nlohmann::json modules = root["modules"];
  size_t i = 0;
  for (const auto &j : modules) {
    uint32_t board_id = j["board_id"];
    if (board_mappings.size() <= board_id)
      board_mappings.resize(board_id + 1, invalid_board_mapping);
    board_mappings[board_id] = i;

    uint8_t SUMO_id = j["SUMO_type"];
    SUMO_mappings.emplace_back(SumoMappings((root_dir / SUMO_mappings_file).string(), SUMO_id));
    i++;
  }

  if (!SUMO_mappings.empty()) {
    /// deduce logical geometry from SUMO mappings
    SumoCoordinates maximum = SUMO_mappings[0].max();
    for (const auto& s : SUMO_mappings) {
      maximum = max(maximum, s.max());
    }
    geometry.nx(maximum.wire_layer + 1);
    geometry.ny(maximum.wire + 1);
    geometry.nz(maximum.strip + 1);
  } else {
    /// if SUMO mappings were not present, must manually provide bounds in json
    nlohmann::json geom = root["geometry"];
    geometry.nx(geom["x"]);
    geometry.ny(geom["y"]);
    geometry.nz(geom["z"]);
  }

  geometry.np(i);

  maximum_latency = root["maximum_latency"];
  merger = ChronoMerger(maximum_latency, i);
}

std::string Config::debug() const {
  std::stringstream ss;
  ss << "  ========           Jalousie Config           ========\n";
  ss << "\n";

  for (size_t i=0; i < board_mappings.size(); ++i) {
    if (board_mappings[i] != invalid_board_mapping) {
      ss << "   board[" << i << "] --> " << board_mappings[i] << "\n";
    }
  }
  ss << "\n";
  for (size_t i=0; i < SUMO_mappings.size(); ++i) {
    ss << "   SUMO[" << i << "] " << SUMO_mappings[i].debug(false) << "\n";
  }
  ss << "\n";
  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";
  ss << "  geometry_p = " << geometry.np() << "\n";
  ss << "  Maximum latency: " << maximum_latency << "\n";

  return ss.str();
}

}
