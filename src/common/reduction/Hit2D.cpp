// Copyright (C) 2023 European Spallation Source ERIC
//===----------------------------------------------------------------------===//
///
/// \file Hit2D.cpp
/// \brief Hit2D implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/Hit2D.h>
#include <fmt/format.h>

std::string Hit2D::to_string() const {
  return fmt::format("time={:>20}  x_coord={:>5}  y_coord={:>5}  weight={:>5}",
                     time, x_coordinate, y_coordinate, weight);
}
