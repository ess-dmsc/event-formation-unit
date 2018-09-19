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

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

int DataParser::parse(const char *buffer, unsigned int size) {

  mbheader = nullptr;
  mbdata = nullptr;
  memset(&stats, 0, sizeof(struct stats));

  auto headerlen = sizeof(struct Header);
  if (size < headerlen) {
    XTRACE(DATA, WAR, "Invalid data size: received %d, min. expected: %lu", size, headerlen);
    stats.error_bytes += size;
    return -error::ESIZE;
  }

  mbheader = (struct Header *)buffer;

  auto expectedsize = sizeof(struct Header) + mbheader->numElements * sizeof(struct ListElement422);

  if (size != expectedsize) {
    XTRACE(DATA, WAR, "Data length mismatch: received %d, expected %lu", size, expectedsize);
    stats.error_bytes += size;
    return -error::ESIZE;
  }

  mbdata = (struct ListElement422 *)(buffer + sizeof(struct Header));

  return mbheader->numElements;
}
