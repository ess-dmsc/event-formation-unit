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
#include <sstream>

#include <limits>

#include <nlohmann/json.hpp>

namespace Multigrid {

struct Filter {
  uint16_t minimum{0};
  uint16_t maximum{std::numeric_limits<uint16_t>::max()};
  double rescale_factor{1.0};

  inline bool trivial() const {
    return ((rescale_factor == 1.0)
        && (minimum == 0)
        && (maximum == std::numeric_limits<uint16_t>::max()));
  }

  inline uint16_t rescale(uint16_t adc) const {
    return static_cast<uint16_t>(std::round(adc * rescale_factor));
  }

  inline bool valid(uint16_t adc) const {
    return ((minimum <= adc) && (adc <= maximum));
  }

  std::string debug() const {
    std::stringstream ss;
    if (minimum != 0)
      ss << "min=" << minimum << "  ";
    if (maximum != std::numeric_limits<uint16_t>::max())
      ss << "max=" << maximum << "  ";
    if (rescale_factor != 1.0)
      ss << "rescale=" << rescale_factor << "  ";
    return ss.str();
  }
};

inline void from_json(const nlohmann::json &j, Filter &f) {
  if (j.count("min"))
    f.minimum = j["min"];
  if (j.count("max"))
    f.maximum = j["max"];
  if (j.count("rescale"))
    f.rescale_factor = j["rescale"];
}

}
