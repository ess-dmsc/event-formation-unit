/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Dataset for running unit tests - do not edit if unsure of what they do!
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <vector>

// clang-format off

std::vector<uint8_t> ErrCookie
{//  'E'   'S'   'R'   v0
    0x45, 0x53, 0x52, 0x00
};

std::vector<uint8_t> ErrVersion
{//  'E'   'S'   'S'   v1
    0x45, 0x53, 0x53, 0x01
};

// must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> OkVersion
{//  'E'   'S'   'S'   v1
    0x45, 0x53, 0x53, 0x00,
    0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// clang-format on
