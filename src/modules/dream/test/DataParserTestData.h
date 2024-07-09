// Copyright (C) 2021 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief test data for DataParserTest
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <vector>

// clang-format off

std::vector<uint8_t> ErrBadRingGoodFEN
{
    0x18, 0x17, 0x04, 0x00, // Data Header, fiber 12, fen 23
};

std::vector<uint8_t> ErrGoodRingBadFEN
{
    0x0b, 0x18, 0x04, 0x00, // Data Header, fiber 11, fen 24
};


std::vector<uint8_t> OkThreeCDTReadouts
{
  // Readout 1
  0x00, 0x01, 0x10, 0x00, // fiber 0, fen 1, data size 40 bytes
  0x22, 0x00, 0x00, 0x00, // tof 34 (0x22)
  0x00, 0x00, 0x14, 0x05, // unused x0000 module 20, sumo 5
  0x0e, 0x0b, 0x01, 0x02, // strip 14, wire 11, segment 1, counter 2

  // Readout 2
  0x00, 0x01, 0x10, 0x00, // fiber 0, fen 1, data size 40 bytes
  0x23, 0x00, 0x10, 0x00, // tof 35 (0x23)
  0x00, 0x00, 0x06, 0x05, // unused 00 00 module 6, sumo 5
  0x0d, 0x08, 0x02, 0x01, // strip 13, wire 8, segment 2, counter 1

  // Readout 3
  0x00, 0x01, 0x10, 0x00, // fiber 0, fen 1, data size 40 bytes
  0x24, 0x00, 0x00, 0x00, // tof 36 (0x24)
  0x00, 0x00, 0x01, 0x05, // unused 00 00 module 1, sumo 5
  0x0b, 0x01, 0x07, 0x01  // strip 11, wire 1, segment 7, counter 1
};

// clang-format on
