/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <netinet/in.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

// clang-format off
// sis3153 and mesytec data types from
// Struck: mvme-src-0.9.2-281-g1c4c24c.tar
// Struck: Ethernet UDP Addendum revision 107
enum SisType : uint32_t {
  BeginReadout = 0xbb000000,
  EndReadout   = 0xee000000
};
// clang-format on

// \todo get rid of magic numbers

size_t Sis3153Parser::parse(Buffer<uint8_t> buffer) {

  buffers.clear();

  if (buffer.buffer[0] != 0x60) {
    XTRACE(DATA, WAR, "Expected buffer header value 0x60");
    return 0;
  }

  if (buffer.size < 19) {
    XTRACE(DATA, WAR, "Buffer too small");
    return 0;
  }

  buffer += 3;
  auto buf32 = Buffer<uint32_t>(reinterpret_cast<uint32_t *>(buffer.buffer), buffer.size/4);

  while (buf32.size > 4) {

    // Header1?
    if ((*buf32.buffer & 0x000000ff) != 0x58) {
      XTRACE(DATA, WAR, "Expected header value 0x58");
      return 0;
    }
    auto length32 = ntohs(static_cast<uint16_t>((*buf32.buffer >> 8) & 0x0000ffff));
    XTRACE(DATA, DEB, "sis3153 datawords %d", length32);
    buf32++;

    // Header2?
    if ((*buf32.buffer & 0xff000000) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "Expected readout header value 0x%04x, got 0x%04x",
             SisType::BeginReadout, (*buf32.buffer & 0xff000000));
      return 0;
    }
    buf32++;

    buffers.emplace_back(Buffer<uint32_t>(buf32.buffer, length32 - 3));

    buf32 += (length32 - 3);

    if (*buf32.buffer != 0x87654321) {
      XTRACE(DATA, WAR, "Protocol mismatch, expected 0x87654321");
      return 0;
    }
    buf32++;

    if ((*buf32.buffer & 0xff000000) != SisType::EndReadout) {
      XTRACE(DATA, WAR, "Missing End-of-readout marker");
      return 0;
    }
    buf32++;
  }

  return buffers.size();
}

}
