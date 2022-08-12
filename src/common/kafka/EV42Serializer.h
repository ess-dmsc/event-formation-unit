// Copyright (C) 2016-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization into ev42 schema
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include "Producer.h"
#include "flatbuffers/flatbuffers.h"

struct EventMessage;

class EV42Serializer {
public:
  /// \brief creates ev42 flat buffer serializer
  /// \param max_array_length maximum number of events
  /// \param source_name value for source_name field
  EV42Serializer(size_t MaxArrayLength, std::string SourceName,
                 ProducerCallback Callback = {});

  /// \brief sets producer callback
  /// \param cb function to be called to send buffer to Kafka
  void setProducerCallback(ProducerCallback Callback);

  /// \brief changes pulse time
  void pulseTime(uint64_t Time);

  /// \returns the currently set pulse time
  uint64_t pulseTime() const;

  /// \brief adds event, if maximum count is exceeded, sends data using the
  /// producer callback \param time time of event in relation to pulse time
  /// \param pixl id of pixel as defined by logical geometry mapping
  /// \returns bytes transmitted, if any
  size_t addEvent(uint32_t Time, uint32_t Pixel);

  /// \brief returns event count
  size_t eventCount() const;

  /// \returns current message counter
  uint64_t currentMessageId() const;

  /// \brief checks if new reference time being used, if so message needs to be
  /// produced
  uint32_t checkAndSetPulseTime(uint64_t Time);

  /// \brief serializes and sends to producer
  /// \returns bytes transmitted
  size_t produce();

  // \todo make private?
  /// \brief serializes buffer
  /// \returns reference to internally stored buffer
  nonstd::span<const uint8_t> serialize();

private:
  // \todo should this not be predefined in terms of jumbo frame?
  size_t MaxEvents{0};
  size_t EventCount{0};

  // \todo maybe should be mutated directly in buffer? Start at 0?
  uint64_t MessageId{1};

  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder Builder_;

  ProducerCallback ProduceFunctor;

  uint8_t *TimePtr{nullptr};
  uint8_t *PixelPtr{nullptr};

  EventMessage *EventMessage_;

  nonstd::span<const uint8_t> Buffer_;
  flatbuffers::uoffset_t *TimeLengthPtr;
  flatbuffers::uoffset_t *PixelLengthPtr;
};
