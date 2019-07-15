/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Hit.cpp
/// \brief Hit implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/Hit.h>
#include <fmt/format.h>

std::string Hit::to_string() const {
  return fmt::format("time={:>20}  plane={:>3}  coord={:>5}  weight={:>5}",
                     time, plane, coordinate, weight);
}
