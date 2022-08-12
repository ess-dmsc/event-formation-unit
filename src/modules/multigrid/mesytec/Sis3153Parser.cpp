/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/Sis3153Parser.h>
#include <netinet/in.h>

#include <common/debug/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

// clang-format off
// sis3153 and mesytec data types from
//    Struck: mvme-src-0.9.2-281-g1c4c24c.tar
//    Struck: Ethernet UDP Addendum revision 107
enum SisType : uint32_t {
  BeginReadout = 0xbb000000,
  EndReadout   = 0xee000000
};

static constexpr uint8_t  Header1       {0x60};

static constexpr uint32_t HeaderMask    {0x000000ff};
static constexpr uint32_t Header2       {0x00000058};
static constexpr uint32_t TypeMask      {0xff000000};
static constexpr uint32_t EndDataCookie {0x87654321};
static constexpr uint32_t LengthMask    {0x0000ffff};
// clang-format on

// \todo get rid of magic numbers

Sis3153Parser::Sis3153Parser() { buffers.reserve(1000); }

size_t Sis3153Parser::parse(Buffer<uint8_t> buffer) {

  buffers.clear();

  if (buffer[0] != Header1) {
    XTRACE(DATA, DEB, "Missing Header1");
    return buffer.bytes();
  }

  if (buffer.size < 19) {
    XTRACE(DATA, WAR, "Buffer too small");
    return buffer.bytes();
  }

  buffer += 3;
  auto buf32 = Buffer<uint32_t>(buffer);

  while (buf32.size > 4) {

    // Header1?
    if ((buf32[0] & HeaderMask) != Header2) {
      XTRACE(DATA, DEB, "Missing Header2");
      return buf32.bytes();
    }
    auto length32 = ntohs(static_cast<uint16_t>((buf32[0] >> 8) & LengthMask));
    XTRACE(DATA, DEB, "sis3153 datawords %d", length32);
    buf32++;

    // Header2?
    if ((buf32[0] & TypeMask) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "Expected readout header value 0x%04x, got 0x%04x",
             SisType::BeginReadout, (buf32[0] & TypeMask));
      return buf32.bytes();
    }
    buf32++;

    length32 -= 3;
    Buffer<uint32_t> sub_buffer(buf32.address, length32);

    buf32 += length32;

    if (buf32[0] != EndDataCookie) {
      XTRACE(DATA, WAR, "Protocol mismatch, end-of-data cookie missing");
      return buf32.bytes();
    }
    buf32++;

    if ((buf32[0] & TypeMask) != SisType::EndReadout) {
      XTRACE(DATA, WAR, "Missing end-of-readout marker");
      return buf32.bytes();
    }
    buf32++;

    buffers.emplace_back(sub_buffer);
  }

  return 0;
}

} // namespace Multigrid
