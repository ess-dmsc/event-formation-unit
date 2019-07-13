/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 */

#pragma once
#include <common/clustering/Hit.h>

namespace Multigrid {

static constexpr uint8_t wire_plane{0};
static constexpr uint8_t grid_plane{1};

inline size_t module_from_plane(uint8_t plane) {
  return plane / 2;
}

inline size_t plane_in_module(uint8_t plane) {
  return plane % 2;
}

}

