/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/FBSerializer.h>
#include <libs/include/gccintel.h>
#include <common/Trace.h>
#include <cinttypes>

#define TIMESIZE 4
#define PIXELSIZE 4

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static_assert(FLATBUFFERS_LITTLEENDIAN, "Flatbuffers only tested on little endian systems");

FBSerializer::FBSerializer(size_t maxarraylength, Producer & prod)
    : builder(maxarraylength * 8 + 256)
    , maxlen(maxarraylength)
    , producer(prod) {
      auto timeoff = builder.CreateUninitializedVector(maxlen, TIMESIZE, &timeptr);
      auto pixeloff = builder.CreateUninitializedVector(maxlen, PIXELSIZE, &pixelptr);
      auto sourceName = builder.CreateString("n/a");
      std::uint64_t sequenceNr = 1; //Placeholder, must not be 0
      std::uint64_t pulseTime = 1; //Placeholder, must not be 0
      auto evMsgHeader = CreateEventMessage(builder, sourceName, sequenceNr, pulseTime, timeoff, pixeloff);
      builder.Finish(evMsgHeader);
      fbBufferPointer = builder.GetBufferPointer();
      fbSize = builder.GetSize();
      
      eventMsg = const_cast<EventMessage*>(GetEventMessage(fbBufferPointer));
      timeLenPtr = reinterpret_cast<flatbuffers::uoffset_t*>(events->time_of_flight()->Data() - 1);
      pixelLenPtr = reinterpret_cast<flatbuffers::uoffset_t*>(events->detector_id()->Data() - 1);
    }

FBSerializer::~FBSerializer() {}

int FBSerializer::serialize(uint64_t time, uint64_t seqno, size_t entries, char **buffer) {
  if (entries > maxlen) {
    *buffer = 0;
    return 0;
  }
  eventMsg->mutate_pulse_time(time);
  eventMsg->mutate_message_id(seqno);
  *timeLenPtr = entries;
  *pixelLenPtr = entries;
  
  *buffer = fbBufferPointer;
  auto sz = fbSize;
  assert(sz > 0);
  assert(buffer != nullptr);
  return sz;
}


int FBSerializer::produce() {
    int txlen = 0;
    if (events != 0) {
      // Debug
      for (unsigned int i = 0; i < events; i++) {
        printf("(%d, %d) ", i, ((uint32_t *)pixelptr)[i]);
      }
      printf("\n");

      char *txbuffer;
      XTRACE(OUTPUT, DEB, "forced produce %zu events \n", events);
      txlen = serialize((uint64_t)0x01, seqno++, events, &txbuffer);
      assert(txlen > 0);
      XTRACE(OUTPUT, DEB, "Flatbuffer tx length %d\n", txlen);
      producer.produce(txbuffer, txlen);
      events = 0;
    }
    return txlen;
}

int FBSerializer::addevent(uint32_t time, uint32_t pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", time, pixel);
  ((uint32_t *)timeptr)[events] = time;
  ((uint32_t *)pixelptr)[events] = pixel;
  events++;

  /** Produce when enough data has been accumulated */
  if (events >= maxlen) {
    return produce();
  }
  return 0;
}
