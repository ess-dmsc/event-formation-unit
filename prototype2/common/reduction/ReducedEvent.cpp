/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file ReducedEvent.h
/// \brief ReducedHit and ReducedEvent class definitions
///
//===----------------------------------------------------------------------===//

#include <common/reduction/ReducedEvent.h>
#include <fmt/format.h>

uint32_t ReducedHit::center_rounded() const {
  return static_cast<uint32_t>(std::round(center));
}

bool ReducedHit::is_center_good() const {
  return std::isfinite(center) && (center >= 0);
}

std::string ReducedHit::to_string() const {
  return fmt::format("{}(lu={},uu={}) t={}(~{}) *{}",
                     center, uncert_lower, uncert_upper, average_time, time, hits_used);
}

std::string ReducedEvent::to_string() const {
  return fmt::format("x=[{}]   y=[{}]   z=[{}]   t={}   {}",
                     x.to_string(), y.to_string(), z.to_string(),
                     time, (good ? "good" : "bad"));
}
