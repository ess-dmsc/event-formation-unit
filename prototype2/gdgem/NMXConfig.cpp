/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <gdgem/NMXConfig.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Gem {

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

  /**< \todo get from slow control? */
  if (builder_type == "VMM3") {
    auto tc = root["time_config"];
    time_config.tac_slope_ns(tc["tac_slope"].get<int>());
    time_config.bc_clock_MHz(tc["bc_clock"].get<int>());
    time_config.trigger_resolution_ns(tc["trigger_resolution"].get<double>());
    time_config.acquisition_window(tc["acquisition_window"].get<unsigned int>());
  }

  if (builder_type != "Hits") {
    auto sm = root["srs_mappings"];
    for (unsigned int index = 0; index < sm.size(); index++) {
      auto fecID = sm[index]["fecID"].get<int>();
      auto vmmID = sm[index]["vmmID"].get<int>();
      auto planeID = sm[index]["planeID"].get<int>();
      auto strip_offset = sm[index]["strip_offset"].get<int>();
      srs_mappings.set_mapping(fecID, vmmID, planeID, strip_offset);
    }

    adc_threshold = root["adc_threshold"].get<unsigned int>();
  }

  hit_histograms = root["hit_histograms"].get<bool>();

  perform_clustering = root["perform_clustering"].get<bool>();

  if (perform_clustering) {
    auto cx = root["clusterer x"];
    clusterer_x.max_strip_gap = cx["max_strip_gap"].get<unsigned int>();
    clusterer_x.max_time_gap = cx["max_time_gap"].get<double>();

    auto cy = root["clusterer y"];
    clusterer_y.max_strip_gap = cy["max_strip_gap"].get<unsigned int>();
    clusterer_y.max_time_gap = cy["max_time_gap"].get<double>();

    matcher_max_delta_time = root["matcher_max_delta_time"].get<double>();

    analyze_weighted = root["analyze_weighted"].get<bool>();
    analyze_max_timebins = root["analyze_max_timebins"].get<unsigned int>();
    analyze_max_timedif = root["analyze_max_timedif"].get<unsigned int>();

    auto f = root["filters"];
    filter.enforce_lower_uncertainty_limit =
            f["enforce_lower_uncertainty_limit"].get<bool>();
    filter.lower_uncertainty_limit = f["lower_uncertainty_limit"].get<unsigned int>();
    filter.enforce_minimum_hits = f["enforce_minimum_hits"].get<bool>();
    filter.minimum_hits = f["minimum_hits"].get<unsigned int>();

    track_sample_minhits = root["track_sample_minhits"].get<unsigned int>();
    cluster_adc_downshift = root["cluster_adc_downshift"].get<unsigned int>();
    send_tracks = root["send_tracks"].get<bool>();

    // \todo deduce geometry from SRS mappings?
    geometry.nx(root["geometry_x"].get<unsigned int>());
    geometry.ny(root["geometry_y"].get<unsigned int>());
    geometry.nz(1);
    geometry.np(1);
  }
}

std::string NMXConfig::debug() const {
  // \todo use fmt
  std::stringstream ss;
  ss << "  ==========================================\n";
  ss << "  ========       builder: "
     << builder_type
     << "      ========\n";
  ss << "  ==========================================\n";
  if (builder_type == "VMM3") {
    ss << "  Time config:\n" << time_config.debug();
  }
  if (builder_type != "Hits") {
    ss << "  Digital geometry:\n" << srs_mappings.debug();
    ss << "\n  adc_threshold = " << adc_threshold << "\n";
  }

  ss << "  Histogram hits = " << (hit_histograms ? "YES" : "no") << "\n";

  ss << "\n";

  ss << "  Perform clustering = " << (perform_clustering ? "YES" : "no") << "\n";

  if (perform_clustering) {
    ss << "  Clusterer-X:\n";
    ss << "    max_time_gap = " << clusterer_x.max_time_gap << "\n";
    ss << "    max_strip_gap = " << clusterer_x.max_strip_gap << "\n";

    ss << "  Clusterer-Y:\n";
    ss << "    max_time_gap = " << clusterer_y.max_time_gap << "\n";
    ss << "    max_strip_gap = " << clusterer_y.max_strip_gap << "\n";

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

    if (hit_histograms)
      ss << "    cluster_adc_downshift = " << cluster_adc_downshift << "\n";
    ss << "  Send tracks = " << (send_tracks ? "YES" : "no") << "\n";
    if (send_tracks)
      ss << "    sample_minhits = " << track_sample_minhits << "\n";

    ss << "  geometry_x = " << geometry.nx() << "\n";
    ss << "  geometry_y = " << geometry.ny() << "\n";
  }

  if (calfile) {
      ss << "\nVMM Calibrations:\n" + calfile->debug();
  }

  return ss.str();
}

}
