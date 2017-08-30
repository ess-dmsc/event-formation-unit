/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Dataset for running unit tests - do not edit if unsure of what they
 * do!
 */

#include <vector>

using namespace std;

// clang-format off

//
// Invalid data
//
vector<uint8_t > err_short_header
{
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

vector<uint8_t> err_version
{// *
  0x21, 0xD6, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};

vector<uint8_t> err_datatype
{//       **
  0x01, 0xD5, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};

vector<uint8_t> err_length
{//                                                        *
  0x01, 0xD6, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01
};

//
// Valid data
//
vector<uint8_t> ok_header_only
{
  0x01, 0xD6, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
};

// clang-format on

/** Raw packet data above, now collect into iterable containers */

vector<vector<uint8_t>> err_hdr{err_short_header, err_version, err_datatype, err_length};
