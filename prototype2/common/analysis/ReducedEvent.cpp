/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/analysis/ReducedEvent.h>
#include <fmt/format.h>

uint32_t ReducedHit::center_rounded() const {
  return static_cast<uint32_t>(std::round(center));
}

bool ReducedHit::is_center_good() const {
  return std::isfinite(center) && (center >= 0);
}

std::string ReducedHit::debug() const {
  return fmt::format("{}(lu={},uu={}) t={}(~{}) *{}",
      center, uncert_lower, uncert_upper, average_time, time, hits_used);
}

std::string ReducedEvent::debug() const {
  return fmt::format("m={} x=[{}], y=[{}], z=[{}], t={} {}",
      module, x.debug(), y.debug(), z.debug(),
      time, (good ? "good" : "bad"));
}

size_t ReducedEvent::hits_used() const {
  return x.hits_used + y.hits_used + z.hits_used;
}
