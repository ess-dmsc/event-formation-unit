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
#include <caen/DataParser.h>
#include <memory>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

int DataParser::parse(const char *buffer, unsigned int size) {

  readouts.clear();
  MBHeader = nullptr;
  Data = nullptr;
  Stats = {};

  auto headerlen = sizeof(struct Header);
  if (size < headerlen) {
    XTRACE(DATA, WAR, "Invalid data size: received %d, min. expected: %lu", size, headerlen);
    Stats.error_bytes += size;
    return -error::ESIZE;
  }

  MBHeader = (struct Header *)buffer;

  if (MBHeader->version != Version) {
    XTRACE(DATA, WAR, "Unsupported data version: 0x%04x (expected 0x%04x)", MBHeader->version, Version);
    Stats.error_version++;
    return -error::EHEADER;
  }

  if (MBHeader->elementType != ElementType) {
    XTRACE(DATA, WAR, "Unsupported data type: 0x%04x (expected 0x%04x)", MBHeader->elementType, ElementType);
    Stats.error_bytes += size;
    return -error::EHEADER;
  }

  if ((MBHeader->seqNum - PreviousSeqNum) != 1) {
    XTRACE(DATA, WAR, "Sequence number inconsistency: current=%lu, previous=%lu",
        MBHeader->seqNum, PreviousSeqNum);
    Stats.seq_errors++;
    // But we continue anyways
  }
  PreviousSeqNum = MBHeader->seqNum;

  auto expectedsize = sizeof(struct Header) + MBHeader->numElements * sizeof(struct ListElement422);

  if (size != expectedsize) {
    XTRACE(DATA, WAR, "Data length mismatch: received %d, expected %lu", size, expectedsize);
    Stats.error_bytes += size;
    return -error::ESIZE;
  }

  Data = (struct ListElement422 *)(buffer + sizeof(struct Header));

  prototype.global_time = MBHeader->globalTime;
  prototype.digitizer = MBHeader->digitizerID;
  readouts.resize(MBHeader->numElements, prototype);
  for (size_t i=0; i < MBHeader->numElements; ++i) {
    auto& r = readouts[i];
    const auto& d = Data[i];
    r.local_time = d.localTime;
    r.channel = d.channel;
    r.adc = d.adcValue;
    //XTRACE(DATA, DEB, "readout %s", r.to_string().c_str());
  }

  return MBHeader->numElements;
}

}
