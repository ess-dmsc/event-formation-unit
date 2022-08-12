/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <cinttypes>
#include <string>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

namespace Multigrid {

struct Filter {
  uint16_t minimum{0};
  uint16_t maximum{std::numeric_limits<uint16_t>::max()};
  double rescale_factor{1.0};

  bool trivial() const;

  uint16_t rescale(uint16_t adc) const;

  bool valid(uint16_t adc) const;

  std::string debug() const;
};

void from_json(const nlohmann::json &j, Filter &f);

class FilterSet {
protected:
  std::vector<Filter> filters_;

public:
  uint16_t rescale(uint16_t coord, uint16_t adc) const;

  bool valid(uint16_t coord, uint16_t adc) const;

  void set_filters(size_t channels, Filter mgf);

  void override_filter(uint16_t coord, Filter mgf);

  std::string debug(std::string prefix) const;
};

void from_json(const nlohmann::json &j, FilterSet &g);

}
