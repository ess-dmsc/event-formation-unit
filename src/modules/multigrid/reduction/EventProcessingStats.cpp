/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EventProcessingStats.h>
#include <sstream>

namespace Multigrid {

void EventProcessingStats::clear() {
  invalid_planes = 0;
  time_seq_errors = 0;
  wire_clusters = 0;
  grid_clusters = 0;
  events_total = 0;
  events_multiplicity_rejects = 0;
  hits_used = 0;
  events_bad = 0;
  events_geometry_err = 0;
}

EventProcessingStats &EventProcessingStats::operator+=(const EventProcessingStats &other) {
  invalid_planes += other.invalid_planes;
  time_seq_errors += other.time_seq_errors;
  wire_clusters += other.wire_clusters;
  grid_clusters += other.grid_clusters;
  events_total += other.events_total;
  events_multiplicity_rejects += other.events_multiplicity_rejects;
  hits_used += other.hits_used;
  events_bad += other.events_bad;
  events_geometry_err += other.events_geometry_err;
  return *this;
}

std::string EventProcessingStats::debug(std::string prepend) const {
  std::stringstream ss;
  ss << prepend << "invalid_planes: " << invalid_planes << "\n";
  ss << prepend << "time_seq_errors: " << time_seq_errors << "\n";
  ss << prepend << "wire_clusters: " << wire_clusters << "\n";
  ss << prepend << "grid_clusters: " << grid_clusters << "\n";
  ss << prepend << "events_total: " << events_total << "\n";
  ss << prepend << "events_multiplicity_rejects: " << events_multiplicity_rejects << "\n";
  ss << prepend << "hits_used: " << hits_used << "\n";
  ss << prepend << "events_bad: " << events_bad << "\n";
  ss << prepend << "events_geometry_err: " << events_geometry_err << "\n";
  return ss.str();
}

}