/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/NMXConfig.h>

#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <sstream>

NMXConfig::NMXConfig(std::string jsonfile)
{
  Json::Value root{};
  Json::Reader reader{};
  std::ifstream t(jsonfile);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (!reader.parse(str, root, 0))
  {
//    ss << "error: file " << jsonfile
//              << " is not valid json\n";
    return;
  }

  builder_type = root["builder_type"].asString();

  if (builder_type == "SRS")
  {
    /**< @todo get from slow control? */
    auto tc = root["time_config"];
    time_config.set_tac_slope(tc["tac_slope"].asInt());
    time_config.set_bc_clock(tc["bc_clock"].asInt());
    time_config.set_trigger_resolution(tc["trigger_resolution"].asDouble());
    time_config.set_target_resolution(tc["target_resolution"].asDouble());

    auto sm = root["srs_mappings"];
    for (unsigned int index = 0; index < sm.size(); index++) {
      auto fecID = sm[index]["fecID"].asInt();
      auto vmmID = sm[index]["vmmID"].asInt();
      auto planeID = sm[index]["planeID"].asInt();
      auto strip_offset = sm[index]["strip_offset"].asInt();
      srs_mappings.set_mapping(fecID, vmmID, planeID, strip_offset);
    }
  }

  cluster_min_timespan = root["cluster_min_timespan"].asInt();
  analyze_weighted = root["analyze_weighted"].asBool();
  analyze_max_timebins = root["analyze_max_timebins"].asInt();
  analyze_max_timedif = root["analyze_max_timedif"].asInt();

  enforce_lower_uncertainty_limit = root["enforce_lower_uncertainty_limit"].asBool();
  lower_uncertainty_limit = root["lower_uncertainty_limit"].asInt();
  enforce_minimum_eventlets = root["enforce_minimum_eventlets"].asBool();
  minimum_eventlets = root["minimum_eventlets"].asInt();

  track_sample_minhits = root["track_sample_minhits"].asInt();
  cluster_adc_downshift = root["cluster_adc_downshift"].asInt();

  geometry_x = root["geometry_x"].asInt();
  geometry_y = root["geometry_y"].asInt();
}

std::string NMXConfig::debug() const
{
  std::stringstream ss;
  ss << "  builder_type = " << builder_type << "\n";
  if (builder_type == "SRS")
  {
    ss << "  time = " << time_config.debug() << "\n";
    ss << "  mappings:\n" << srs_mappings.debug();
  }
  ss << "  cluster_min_timespan = " << cluster_min_timespan << "\n";
  ss << "  analyze_weighted = "
     << (analyze_weighted ? "true" : "false") << "\n";
  ss << "  analyze_max_timebins = " << analyze_max_timebins << "\n";
  ss << "  analyze_max_timedif = " << analyze_max_timedif << "\n";

  ss << "  enforce_lower_uncertainty_limit = "
     << (enforce_lower_uncertainty_limit ? "true" : "false") << "\n";
  ss << "  lower_uncertainty_limit = " << lower_uncertainty_limit << "\n";
  ss << "  enforce_minimum_eventlets = "
     << (enforce_minimum_eventlets ? "true" : "false") << "\n";
  ss << "  minimum_eventlets = " << minimum_eventlets << "\n";

  ss << "  track_sample_minhits = " << track_sample_minhits << "\n";
  ss << "  cluster_adc_downshift = " << cluster_adc_downshift << "\n";

  ss << "  geometry_x = " << geometry_x << "\n";
  ss << "  geometry_y = " << geometry_y << "\n";
  return ss.str();
}
