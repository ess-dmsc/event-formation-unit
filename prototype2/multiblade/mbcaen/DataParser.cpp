/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for Multi-Blade using Jadaq UDP
/// Data format is described in
/// https://github.com/ess-dmsc/jadaq/blob/devel/DataFormat.hpp
///
//===----------------------------------------------------------------------===//

#include <common/Trace.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mbcaen/DataParser.h>
#include <memory>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

int DataParser::parse(const char *buffer, unsigned int size) {

  MBHeader = nullptr;
  MBData = nullptr;
  memset(&Stats, 0, sizeof(struct Stats));

  auto headerlen = sizeof(struct Header);
  if (size < headerlen) {
    XTRACE(DATA, WAR, "Invalid data size: received %d, min. expected: %lu", size, headerlen);
    Stats.error_bytes += size;
    return -error::ESIZE;
  }

  MBHeader = (struct Header *)buffer;

  if (MBHeader->version != Version) {
    XTRACE(DATA, WAR, "Unsupported data version: 0x%04x (expected 0x04%x)\n", MBHeader->version, Version);
    return -error::EHEADER;
  }

  if (MBHeader->elementType != ElementType) {
    XTRACE(DATA, WAR, "Unsupported data type: 0x%04x (expected 0x04%x)\n", MBHeader->elementType, ElementType);
    return -error::EHEADER;
  }

  auto expectedsize = sizeof(struct Header) + MBHeader->numElements * sizeof(struct ListElement422);

  if (size != expectedsize) {
    XTRACE(DATA, WAR, "Data length mismatch: received %d, expected %lu", size, expectedsize);
    Stats.error_bytes += size;
    return -error::ESIZE;
  }

  MBData = (struct ListElement422 *)(buffer + sizeof(struct Header));

  return MBHeader->numElements;
}

}
