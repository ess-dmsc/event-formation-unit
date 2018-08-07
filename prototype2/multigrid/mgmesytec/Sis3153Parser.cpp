/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <netinet/in.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

// clang-format off
// sis3153 and mesytec data types from
// Struck: mvme-src-0.9.2-281-g1c4c24c.tar
// Struck: Ethernet UDP Addendum revision 107
enum SisType : uint32_t {
  BeginReadout = 0xbb000000,
  EndReadout = 0xee000000
};
// clang-format on


Sis3153Parser::error Sis3153Parser::parse(const Buffer& buffer) {

  buffers.clear();

  auto bytesleft = buffer.size;

  if (buffer.buffer[0] != 0x60) {
    return error::EUNSUPP;
  }

  if (buffer.size < 19) {
    return error::ESIZE;
  }

  uint32_t *datap = reinterpret_cast<uint32_t*>(buffer.buffer + 3);
  bytesleft -= 3;

  while (bytesleft > 16) {

    // Header1?
    if ((*datap & 0x000000ff) != 0x58) {
      XTRACE(DATA, WAR, "expected data value 0x58");
      return error::EUNSUPP;
    }
    uint16_t len = ntohs((*datap & 0x00ffff00) >> 8);
    XTRACE(DATA, DEB, "sis3153 datawords %d", len);
    datap++;
    bytesleft -= 4;

    // Header2?
    if ((*datap & 0xff000000) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "expected readout header value 0x%04x, got 0x%04x",
             SisType::BeginReadout, (*datap & 0xff000000));
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    buffers.emplace_back(Buffer(datap, len - 3));

    datap += (len - 3);
    bytesleft -= (len - 3) * 4;

    if (*datap != 0x87654321) {
      XTRACE(DATA, WAR, "Protocol mismatch, expected 0x87654321");
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != SisType::EndReadout) {
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
  }


  return error::OK;
}
