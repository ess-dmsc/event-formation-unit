// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Test data for JalousieBaseTest
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>

std::string JalousieConfig = R"(
{
  "SUMO_mappings_file" : "../configs/sumo_voxel_map_20190711.csv",
  "modules" :
  [
    {"board_id":1418045, "SUMO_type":3},
    {"board_id":1416964, "SUMO_type":4},
    {"board_id":1416799, "SUMO_type":5},
    {"board_id":1416697, "SUMO_type":6}
  ],

  "geometry": {
    "x": 64,
    "y": 128,
    "z": 1
  },

  "maximum_latency" : 30000000
}
)";
// \todo get rid of this file; use binary reference data instead

// For now, just invalid data
std::vector<uint8_t> DummyJalousieData = {
// valid board id 0x15A33D
//****  ****  ****  ****
  0x3d, 0xa3, 0x15, 0x00, 0x01, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,

// board id 0x15A33D      **** chopper sub id
  0x3d, 0xa3, 0x15, 0x00, 0xff, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,

// bad board id
  0xba, 0xd0, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,

// valid board id 0x15A33D
//****  ****  ****  ****
  0x3d, 0xa3, 0x15, 0x00, 0x01, 0x01, 0x00, 0x00,
// invalid cathode id                 ****
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff,
};
