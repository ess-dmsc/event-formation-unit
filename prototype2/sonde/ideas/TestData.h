/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Dataset for running unit tests - do not edit if unsure of what they
 * do!
 */

#include <vector>

using namespace std;

// clang-format off

vector<unsigned int> err_short_header
{
  0x01020304, 0x05060707
};

vector<unsigned int> ok_header_only
{
  0x01020304, 0x05060708, 0x090a0b0c
};

// clang-format on

/** Raw packet data above, now collect into iterable containers */

//vector<vector<unsigned int>> ok{ok_one, ok_two, ok_16};
