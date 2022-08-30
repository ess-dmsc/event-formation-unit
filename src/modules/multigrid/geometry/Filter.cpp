/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/Filter.h>
#include <sstream>

namespace Multigrid {

bool Filter::trivial() const {
  return ((rescale_factor == 1.0) && (minimum == 0) &&
          (maximum == std::numeric_limits<uint16_t>::max()));
}

uint16_t Filter::rescale(uint16_t adc) const {
  return static_cast<uint16_t>(std::round(adc * rescale_factor));
}

bool Filter::valid(uint16_t adc) const {
  return ((minimum <= adc) && (adc <= maximum));
}

std::string Filter::debug() const {
  std::stringstream ss;
  if (minimum != 0)
    ss << "min=" << minimum << "  ";
  if (maximum != std::numeric_limits<uint16_t>::max())
    ss << "max=" << maximum << "  ";
  if (rescale_factor != 1.0)
    ss << "rescale=" << rescale_factor << "  ";
  return ss.str();
}

void from_json(const nlohmann::json &j, Filter &f) {
  if (j.count("min"))
    f.minimum = j["min"];
  if (j.count("max"))
    f.maximum = j["max"];
  if (j.count("rescale"))
    f.rescale_factor = j["rescale"];
}

uint16_t FilterSet::rescale(uint16_t coord, uint16_t adc) const {
  if (coord >= filters_.size())
    return adc;
  return filters_[coord].rescale(adc);
}

bool FilterSet::valid(uint16_t coord, uint16_t adc) const {
  if (coord >= filters_.size())
    return true;
  return filters_[coord].valid(adc);
}

void FilterSet::set_filters(size_t channels, Filter mgf) {
  filters_.resize(channels);
  std::fill(filters_.begin(), filters_.end(), mgf);
}

void FilterSet::override_filter(uint16_t n, Filter mgf) {
  if (filters_.size() <= n)
    filters_.resize(n + 1);
  filters_[n] = mgf;
}

std::string FilterSet::debug(std::string prefix) const {
  bool validwf{false};
  std::stringstream wfilters;
  for (size_t i = 0; i < filters_.size(); i++) {
    const auto &f = filters_[i];
    if (!f.trivial()) {
      wfilters << prefix << "[" << i << "]  " << f.debug() << "\n";
      validwf = true;
    }
  }

  if (validwf) {
    return wfilters.str();
  }

  return {};
}

void from_json(const nlohmann::json &j, FilterSet &g) {
  if (j.count("blanket")) {
    auto b = j["blanket"];
    g.set_filters(b["count"], b);
  }
  if (j.count("exceptions")) {
    auto wfe = j["exceptions"];
    for (auto k = 0u; k < wfe.size(); k++) {
      g.override_filter(wfe[k]["idx"], wfe[k]);
    }
  }
}

} // namespace Multigrid
