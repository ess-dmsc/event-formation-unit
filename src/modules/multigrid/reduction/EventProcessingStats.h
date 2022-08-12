/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <cstdint>
#include <string>

namespace Multigrid {

struct EventProcessingStats {
  size_t invalid_modules{0};
  size_t invalid_planes{0};
  size_t time_seq_errors{0};
  size_t wire_clusters{0};
  size_t grid_clusters{0};
  size_t events_total{0};
  size_t events_multiplicity_rejects{0};
  size_t hits_used{0};
  size_t events_bad{0};
  size_t events_geometry_err{0};

  void clear();
  EventProcessingStats& operator +=(const EventProcessingStats& other);
  std::string debug(std::string prepend) const;
};

}