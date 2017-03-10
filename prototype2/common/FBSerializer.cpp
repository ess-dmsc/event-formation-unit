/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/FBSerializer.h>
#include <libs/include/gccintel.h>
#include <common/Trace.h>
#include <cinttypes>

#define TIMESIZE 4
#define PIXELSIZE 4

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

static_assert(FLATBUFFERS_LITTLEENDIAN, "Flatbuffers only tested on little endian systems");

FBSerializer::FBSerializer(size_t maxarraylength, Producer & prod)
    : builder(maxarraylength * 8 + 256)
    , maxlen(maxarraylength)
    , producer(prod) {
      builder.Clear();
      builder.CreateUninitializedVector(maxlen, TIMESIZE, &timeptr);
      builder.CreateUninitializedVector(maxlen, PIXELSIZE, &pixelptr);
    }

FBSerializer::~FBSerializer() {}

int FBSerializer::serialize(uint64_t time, uint64_t seqno, size_t entries, char **buffer) {
  if (entries > maxlen) {
    *buffer = 0;
    return 0;
  }

  builder.Clear();

  auto timeoff = builder.CreateUninitializedVector(entries, TIMESIZE, &timeptr);

  auto pixeloff = builder.CreateUninitializedVector(entries, PIXELSIZE, &pixelptr);

  auto msg = CreateEventMessage(builder, 0, seqno, time, timeoff, pixeloff);

  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  return builder.GetSize();
}

int FBSerializer::addevent(uint32_t time, uint32_t pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %d\n", time, pixel);
  ((uint32_t *)timeptr)[events] = time;
  ((uint32_t *)pixelptr)[events] = pixel;
  events++;

  /** Produce when enough data has been accumulated */
  int txlen = 0;
  if (events >= maxlen) {
    char *txbuffer;
    XTRACE(OUTPUT, DEB, "produce %zu events \n", events);
    txlen = serialize((uint64_t)0x01, seqno++, events, &txbuffer);
    XTRACE(OUTPUT, DEB, "Flatbuffer tx length %d\n", txlen);
    producer.produce(txbuffer, txlen);
    events = 0;
  }
  return txlen;
}
