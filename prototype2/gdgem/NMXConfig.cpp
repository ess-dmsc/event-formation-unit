/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <gdgem/NMXConfig.h>

#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <sstream>

NMXConfig::NMXConfig(std::string jsonfile) {
  Json::Value root{};
  Json::Reader reader{};
  std::ifstream t(jsonfile);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (!reader.parse(str, root, 0)) {
    XTRACE(INIT, WAR, "Invalid Json file: %s\n", jsonfile.c_str());
    return;
  }

  builder_type = root["builder_type"].asString();

  if ((builder_type == "VMM2") || (builder_type == "VMM3")) {
    /**< @todo get from slow control? */
    // TODO: decimate tdc bug?
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

  auto cx = root["clusterer x"];
  clusterer_x.eventlet_adc_threshold = cx["eventlet_adc_threshold"].asUInt();
  clusterer_x.max_strip_gap = cx["max_strip_gap"].asUInt();
  clusterer_x.max_time_gap = cx["max_time_gap"].asDouble();
  clusterer_x.min_cluster_size = cx["min_cluster_size"].asLargestUInt();

  auto cy = root["clusterer y"];
  clusterer_y.eventlet_adc_threshold = cy["eventlet_adc_threshold"].asUInt();
  clusterer_y.max_strip_gap = cy["max_strip_gap"].asUInt();
  clusterer_y.max_time_gap = cy["max_time_gap"].asDouble();
  clusterer_y.min_cluster_size = cy["min_cluster_size"].asLargestUInt();

  matcher_max_delta_time = root["matcher_max_delta_time"].asDouble();

  analyze_weighted = root["analyze_weighted"].asBool();
  analyze_max_timebins = root["analyze_max_timebins"].asInt();
  analyze_max_timedif = root["analyze_max_timedif"].asInt();

  auto f = root["filters"];
  filter.enforce_lower_uncertainty_limit =
      f["enforce_lower_uncertainty_limit"].asBool();
  filter.lower_uncertainty_limit = f["lower_uncertainty_limit"].asInt();
  filter.enforce_minimum_eventlets = f["enforce_minimum_eventlets"].asBool();
  filter.minimum_eventlets = f["minimum_eventlets"].asInt();


  eventlet_histograms = root["eventlet_histograms"].asBool();
  track_sample_minhits = root["track_sample_minhits"].asInt();
  cluster_adc_downshift = root["cluster_adc_downshift"].asInt();
  send_tracks = root["send_tracks"].asBool();

  // TODO deduce geometry from SRS mappings?
  geometry.nx(root["geometry_x"].asInt());
  geometry.ny(root["geometry_y"].asInt());
  geometry.nz(1);
  geometry.np(1);

  dump_csv = root["dump_csv"].asBool();
  dump_h5 = root["dump_h5"].asBool();
  dump_directory = root["dump_directory"].asString();
}

std::string NMXConfig::debug() const {
  std::stringstream ss;
  ss << "  =====================================\n";
  ss << "  =====     builder:  "
     << builder_type
     << "     =====\n";
  ss << "  =====================================\n";
  if ((builder_type == "VMM2") || (builder_type == "VMM3")) {
    ss << "  time = " << time_config.debug() << "\n";
    ss << "  Chip geometry:\n" << srs_mappings.debug();
  }

  ss << "  Clusterer-X:\n";
  ss << "    eventlet_adc_threshold = " << clusterer_x.eventlet_adc_threshold << "\n";
  ss << "    max_time_gap = " << clusterer_x.max_time_gap << "\n";
  ss << "    max_strip_gap = " << clusterer_x.max_strip_gap<< "\n";
  ss << "    min_cluster_size = " << clusterer_x.min_cluster_size << "\n";

  ss << "  Clusterer-Y:\n";
  ss << "    eventlet_adc_threshold = " << clusterer_y.eventlet_adc_threshold << "\n";
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
  ss << "    enforce_minimum_eventlets = "
     << (filter.enforce_minimum_eventlets ? "YES" : "no") << "\n";
  if (filter.enforce_minimum_eventlets)
    ss << "    minimum_eventlets = " << filter.minimum_eventlets << "\n";

  ss << "  Histogram eventlets = " << (eventlet_histograms ? "YES" : "no") << "\n";
  if (eventlet_histograms)
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
