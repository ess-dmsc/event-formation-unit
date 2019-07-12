/** Copyright (C) 2017 European Spallation Source ERIC */

#include <jalousie/Config.h>
#include <sstream>
#include <nlohmann/json.hpp>
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

//  // \todo deduce geometry from SUMO mappings instead
  nlohmann::json geom = root["geometry"];
  geometry.nx(geom["x"]);
  geometry.ny(geom["y"]);
  geometry.nz(geom["z"]);
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
      ss << "    board[" << i << "] --> " << board_mappings[i] << "\n";
    }
  }
  ss << "\n";
  for (size_t i=0; i < SUMO_mappings.size(); ++i) {
    ss << "   SUMO[" << i << "]:\n" << SUMO_mappings[i].debug() << "\n";
  }
  ss << "\n";
  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";
  ss << "  geometry_p = " << geometry.np() << "\n";
  ss << "  Maximum latency: " << maximum_latency;

  return ss.str();
}

}
