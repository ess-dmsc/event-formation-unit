/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <gdgem/NMXConfig.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

NMXConfig::NMXConfig(std::string configfile, std::string calibrationfile) {

  calfile = std::make_shared<CalibrationFile>(calibrationfile);

  nlohmann::json root;

  std::ifstream t(configfile);
  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  try {
    root = nlohmann::json::parse(jsonstring);
  }
  catch (...) {
    XTRACE(INIT, WAR, "Invalid Json file: %s", configfile.c_str());
    return;
  }

  builder_type = root["builder_type"].get<std::string>();

  if ((builder_type == "VMM2") || (builder_type == "VMM3")) {
    /**< \todo get from slow control? */
    auto tc = root["time_config"];
    time_config.set_tac_slope(tc["tac_slope"].get<int>());
    time_config.set_bc_clock(tc["bc_clock"].get<int>());
    time_config.set_trigger_resolution_ns(tc["trigger_resolution"].get<double>());
    time_config.set_target_resolution_ns(tc["target_resolution"].get<double>());
    time_config.set_acquisition_window(tc["acquisition_window"].get<unsigned int>());

    auto sm = root["srs_mappings"];
    for (unsigned int index = 0; index < sm.size(); index++) {
      auto fecID = sm[index]["fecID"].get<int>();
      auto vmmID = sm[index]["vmmID"].get<int>();
      auto planeID = sm[index]["planeID"].get<int>();
      auto strip_offset = sm[index]["strip_offset"].get<int>();
      srs_mappings.set_mapping(fecID, vmmID, planeID, strip_offset);
    }
  }

  auto cx = root["clusterer x"];
  clusterer_x.hit_adc_threshold = cx["hit_adc_threshold"].get<unsigned int>();
  clusterer_x.max_strip_gap = cx["max_strip_gap"].get<unsigned int>();
  clusterer_x.max_time_gap = cx["max_time_gap"].get<double>();
  clusterer_x.min_cluster_size = cx["min_cluster_size"].get<size_t>();

  auto cy = root["clusterer y"];
  clusterer_y.hit_adc_threshold = cy["hit_adc_threshold"].get<unsigned int>();
  clusterer_y.max_strip_gap = cy["max_strip_gap"].get<unsigned int>();
  clusterer_y.max_time_gap = cy["max_time_gap"].get<double>();
  clusterer_y.min_cluster_size = cy["min_cluster_size"].get<size_t>();

  matcher_max_delta_time = root["matcher_max_delta_time"].get<double>();

  analyze_weighted = root["analyze_weighted"].get<bool>();
  analyze_max_timebins = root["analyze_max_timebins"].get<int>();
  analyze_max_timedif = root["analyze_max_timedif"].get<int>();

  auto f = root["filters"];
  filter.enforce_lower_uncertainty_limit =
      f["enforce_lower_uncertainty_limit"].get<bool>();
  filter.lower_uncertainty_limit = f["lower_uncertainty_limit"].get<int>();
  filter.enforce_minimum_hits = f["enforce_minimum_hits"].get<bool>();
  filter.minimum_hits = f["minimum_hits"].get<int>();


  hit_histograms = root["hit_histograms"].get<bool>();
  track_sample_minhits = root["track_sample_minhits"].get<int>();
  cluster_adc_downshift = root["cluster_adc_downshift"].get<int>();
  send_tracks = root["send_tracks"].get<bool>();

  // \todo deduce geometry from SRS mappings?
  geometry.nx(root["geometry_x"].get<int>());
  geometry.ny(root["geometry_y"].get<int>());
  geometry.nz(1);
  geometry.np(1);

  dump_csv = root["dump_csv"].get<bool>();
  dump_h5 = root["dump_h5"].get<bool>();
  dump_directory = root["dump_directory"].get<std::string>();
}

std::string NMXConfig::debug() const {
  std::stringstream ss;
  ss << "  ==========================================\n";
  ss << "  ========       builder: "
     << builder_type
     << "      ========\n";
  ss << "  ==========================================\n";
  if ((builder_type == "VMM2") || (builder_type == "VMM3")) {
    ss << "  Time config:\n" << time_config.debug();
    ss << "  Chip geometry:\n" << srs_mappings.debug();
  }

  ss << "  Clusterer-X:\n";
  ss << "    hit_adc_threshold = " << clusterer_x.hit_adc_threshold << "\n";
  ss << "    max_time_gap = " << clusterer_x.max_time_gap << "\n";
  ss << "    max_strip_gap = " << clusterer_x.max_strip_gap<< "\n";
  ss << "    min_cluster_size = " << clusterer_x.min_cluster_size << "\n";

  ss << "  Clusterer-Y:\n";
  ss << "    hit_adc_threshold = " << clusterer_y.hit_adc_threshold << "\n";
  ss << "    max_time_gap = " << clusterer_y.max_time_gap << "\n";
  ss << "    max_strip_gap = " << clusterer_y.max_strip_gap<< "\n";
  ss << "    min_cluster_size = " << clusterer_y.min_cluster_size << "\n";

  ss << "  Matcher\n    max_delta_time = " << matcher_max_delta_time << "\n";

  ss << "  Event analysis\n";
  ss << "    weighted = " << (analyze_weighted ? "true" : "false") << "\n";
  ss << "    max_timebins = " << analyze_max_timebins << "\n";
  ss << "    max_timedif = " << analyze_max_timedif << "\n";

  ss << "  Filters:\n";
  ss << "    enforce_lower_uncertainty_limit = "
     << (filter.enforce_lower_uncertainty_limit ? "YES" : "no") << "\n";
  if (filter.enforce_lower_uncertainty_limit)
    ss << "    lower_uncertainty_limit = " << filter.lower_uncertainty_limit << "\n";
  ss << "    enforce_minimum_hits = "
     << (filter.enforce_minimum_hits ? "YES" : "no") << "\n";
  if (filter.enforce_minimum_hits)
    ss << "    minimum_hits = " << filter.minimum_hits << "\n";

  ss << "  Histogram hits = " << (hit_histograms ? "YES" : "no") << "\n";
  if (hit_histograms)
    ss << "    cluster_adc_downshift = " << cluster_adc_downshift << "\n";
  ss << "  Send tracks = " << (send_tracks ? "YES" : "no") << "\n";
  if (send_tracks)
    ss << "    sample_minhits = " << track_sample_minhits << "\n";

  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";

  ss << "  Dump csv = " << (dump_csv ? "YES" : "no") << "\n";
  ss << "  Dump h5 = " << (dump_h5 ? "YES" : "no") << "\n";
  if (dump_csv || dump_h5)
    ss << "  dump_directory = " << dump_directory << "\n";
  return ss.str();
}
