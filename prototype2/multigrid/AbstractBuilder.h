/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <common/Buffer.h>
#include <common/clustering/Hit.h>
#include <vector>

namespace Multigrid {

class AbstractBuilder {
public:
  static constexpr uint8_t wire_plane {0};
  static constexpr uint8_t grid_plane {1};
  static constexpr uint8_t external_trigger_plane {99};

  virtual void parse(Buffer<uint8_t> buffer) = 0;

  std::vector<Hit> ConvertedData;

  size_t stats_discarded_bytes{0};
  size_t stats_trigger_count{0};
  size_t stats_readout_filter_rejects{0};
  size_t stats_digital_geom_errors{0};
};

}