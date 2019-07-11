/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <vector>
#include <string>

namespace Jalousie {

class SumoMappings {
public:
  struct SumoCoordinates {
    uint8_t wire_layer;
    uint8_t strip;
    uint8_t wire;

    static constexpr uint8_t kInvalidCoord {255};

    bool is_valid() const;
    std::string debug() const;
  };

  static constexpr SumoCoordinates kInvalidCoordinates
    {SumoCoordinates::kInvalidCoord,
     SumoCoordinates::kInvalidCoord,
     SumoCoordinates::kInvalidCoord};

  SumoMappings(std::string file_name, uint8_t sumo_id);

  void add_mapping(uint8_t anode, uint8_t cathode, SumoCoordinates coord);
  SumoCoordinates map(uint8_t anode, uint8_t cathode) const;

  std::string debug() const;

private:
  std::vector<std::vector<SumoCoordinates>> table_;
};

}

