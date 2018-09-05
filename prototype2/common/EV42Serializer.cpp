/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EV42Serializer.h>
#include "ev42_events_generated.h"
#include <libs/include/gccintel.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static constexpr size_t TimeSize = sizeof(uint32_t);
static constexpr size_t PixelSize = sizeof(uint32_t);

// These must be non-0 because of Flatbuffers being stupid
// If they are initially set to 0, they will not be mutable
static constexpr uint64_t FBMutablePlaceholder = 1;

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

EV42Serializer::EV42Serializer(size_t max_array_length, std::string source_name)
    : maxEvents(max_array_length), builder(maxEvents * 8 + 256) {

  auto sourceName = builder.CreateString(source_name);
  auto timeoff = builder.CreateUninitializedVector(maxEvents, TimeSize, &timePtr);
  auto pixeloff =
      builder.CreateUninitializedVector(maxEvents, PixelSize, &pixelPtr);

  auto evMsgHeader = CreateEventMessage(builder, sourceName, FBMutablePlaceholder,
                                        FBMutablePlaceholder, timeoff, pixeloff);
  FinishEventMessageBuffer(builder, evMsgHeader);

  buffer.address = builder.GetBufferPointer();
  buffer.size = builder.GetSize();
  assert(buffer);

  eventMsg = const_cast<EventMessage *>(GetEventMessage(buffer.address));
  timeLenPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(eventMsg->time_of_flight()->Data())) - 1;
  pixelLenPtr =
      reinterpret_cast<flatbuffers::uoffset_t *>(
          const_cast<std::uint8_t *>(eventMsg->detector_id()->Data())) - 1;

  eventMsg->mutate_message_id(0);
  eventMsg->mutate_pulse_time(0);
}

void EV42Serializer::producerCallback(ProducerCallback callback) {
  callbackFunction = callback;
}

Buffer<uint8_t> EV42Serializer::serialize() {
  if (events > maxEvents) {
    // TODO: this should probably throw instead?
    return {};
  }
  eventMsg->mutate_message_id(messageId);
  *timeLenPtr = events;
  *pixelLenPtr = events;

  // reset counter and increment message counter
  events = 0;
  messageId++;

  return buffer;
}

size_t EV42Serializer::produce() {
  if (events != 0) {
    XTRACE(OUTPUT, DEB, "autoproduce %zu events \n", events);
    auto buffer = serialize();
    if (callbackFunction)
      callbackFunction(buffer);
    return buffer.size;
  }
  return 0;
}

void EV42Serializer::pulseTime(uint64_t time) {
  eventMsg->mutate_pulse_time(time);
}

uint64_t EV42Serializer::pulseTime() const {
  return eventMsg->pulse_time();
}

size_t EV42Serializer::eventCount() const {
  return events;
}

uint64_t EV42Serializer::currentMessageId() const {
  return messageId;
}

size_t EV42Serializer::addEvent(uint32_t time, uint32_t pixel) {
  XTRACE(OUTPUT, DEB, "Add event: %d %u\n", time, pixel);
  ((uint32_t *) timePtr)[events] = time;
  ((uint32_t *) pixelPtr)[events] = pixel;
  events++;

  /** Produce when enough data has been accumulated */
  if (events >= maxEvents) {
    return produce();
  }
  return 0;
}
