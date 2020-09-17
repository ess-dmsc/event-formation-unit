// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Digital geometry for SUMO (Sub-Modules)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>
#include <string>

namespace Jalousie {

struct SumoCoordinates {
  static constexpr uint8_t kInvalidCoord{255};

  uint8_t wire_layer{kInvalidCoord};
  uint8_t strip{kInvalidCoord};
  uint8_t wire{kInvalidCoord};

  bool is_valid() const;
  std::string debug() const;
};

SumoCoordinates min(const SumoCoordinates &, const SumoCoordinates &);
SumoCoordinates max(const SumoCoordinates &, const SumoCoordinates &);

class SumoMappings {
public:
  SumoMappings(std::string file_name, uint8_t sumo_id);

  void add_mapping(uint8_t anode, uint8_t cathode, SumoCoordinates coord);
  SumoCoordinates map(uint8_t anode, uint8_t cathode) const;
  SumoCoordinates min() const;
  SumoCoordinates max() const;
  std::string debug(bool depict_domain) const;

private:
  std::vector<std::vector<SumoCoordinates>> table_;
  SumoCoordinates min_;
  SumoCoordinates max_;
};

}
