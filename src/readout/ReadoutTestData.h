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

// Wrong cookie, good version
std::vector<uint8_t> ErrCookie
{//  'E'   'S'   'R'   v0
    0x45, 0x53, 0x52, 0x00
};

// wrong version, good cookie
std::vector<uint8_t> ErrVersion
{//  'E'   'S'   'S'   v1
    0x45, 0x53, 0x53, 0x01
};

// must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> OkVersion
{
    0x45, 0x53, 0x53, 0x00, //  'E' 'S' 'S' 0x00
    0x30, 0x00, 0x1c, 0x00, // 0x001c = 28
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00  // Seq number 1
};

// clang-format on
