/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/AbstractAnalyzer.h>
#include <cmath>
#include <set>
#include <sstream>

namespace Gem {

uint32_t CoordResult::center_rounded() const {
  return static_cast<uint32_t>(std::round(center));
}

std::string CoordResult::debug() const {
  return fmt::format("{}(lu={},uu={})", center, uncert_lower, uncert_upper);
}

std::string MultiDimResult::debug() const {
  return fmt::format("x={}, y={}, z={}, t={}", x.debug(), y.debug(), z.debug(), time);
}

}
