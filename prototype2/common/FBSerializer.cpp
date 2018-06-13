/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/FBSerializer.h>
#include <common/Trace.h>
#include <common/DataSave.h>
#include <libs/include/gccintel.h>

#define TIMESIZE 4
#define PIXELSIZE 4

#define INITIAL_PULSE_TIME 0

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

FBSerializer::FBSerializer(size_t maxarraylength, Producer &prod)
    : builder(maxarraylength * 8 + 256), maxlen(maxarraylength),
      producer(prod) {

  auto sourceName = builder.CreateString("c_spec_data");
  auto timeoff = builder.CreateUninitializedVector(maxlen, TIMESIZE, &timeptr);
  auto pixeloff =
      builder.CreateUninitializedVector(maxlen, PIXELSIZE, &pixelptr);

  auto evMsgHeader = CreateEventMessage(builder, sourceName, seqno,
                                        INITIAL_PULSE_TIME, timeoff, pixeloff);
  FinishEventMessageBuffer(builder, evMsgHeader);

  fbBufferPointer = reinterpret_cast<char*>(builder.GetBufferPointer());
  fbSize = builder.GetSize();
  assert(fbSize > 0);
  assert(fbBufferPointer != nullptr);

  eventMsg = const_cast<EventMessage *>(GetEventMessage(fbBufferPointer));
  timeLenPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(eventMsg->time_of_flight()->Data())) -
      1;
  pixelLenPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(eventMsg->detector_id()->Data())) -
      1;
}

size_t FBSerializer::serialize(size_t entries,
                            char **buffer) {
  if (entries > maxlen) {
    // TODO: this should probably throw instead?

    *buffer = 0;
    return 0;
  }
  eventMsg->mutate_message_id(seqno);
  *timeLenPtr = entries;
  *pixelLenPtr = entries;

  *buffer = fbBufferPointer;
  return fbSize;
}

size_t FBSerializer::produce() {
  size_t txlen = 0;
  if (events != 0) {
    // Debug
    // for (unsigned int i = 0; i < events; i++) {
    //   printf("(%d, %d) ", i, ((uint32_t *)pixelptr)[i]);
    // }
    // printf("\n");

    XTRACE(OUTPUT, DEB, "produce %zu events \n", events);
    char *txbuffer;
    txlen = serialize(events, &txbuffer);
    seqno++;
    assert(txlen > 0);
    XTRACE(OUTPUT, DEB, "Flatbuffer tx length %zu\n", txlen);
    producer.produce(txbuffer, txlen);

#if 0
      DataSave eventfb("eventfb", txbuffer, txlen);
      sleep(3);
      exit(1);
#endif
    events = 0;
  }
  return txlen;
}

void FBSerializer::set_pulse_time(uint64_t time)
{
  eventMsg->mutate_pulse_time(time);
}

uint64_t FBSerializer::get_pulse_time() const
{
  return eventMsg->pulse_time();
}

size_t FBSerializer::addevent(uint32_t time, uint32_t pixel) {
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
