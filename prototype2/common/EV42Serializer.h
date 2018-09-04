/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "ev42_events_generated.h"
#include "Producer.h"
#include <common/Buffer.h>
#include <functional>

class EV42Serializer {
public:
  /// \brief creates ev42 flat buffer serializer
  /// \param max_array_length maximum number of events
  /// \param source_name value for source_name field
  EV42Serializer(size_t max_array_length, std::string source_name);

  /// \brief sets producer callback
  /// \param cb function to be called to send buffer to Kafka
  void producerCallback(ProducerCallback callback);

  /// \brief changes pulse time
  void pulseTime(uint64_t time);

  /// \brief retrieves pulse time
  uint64_t pulseTime() const;

  /// \brief adds event
  /// \param time time of event in relation to pulse time
  /// \param pixl id of pixel as defined by logical geometry mapping
  size_t addEvent(uint32_t time, uint32_t pixel);

  /// \brief returns event count
  size_t eventCount() const;

  /// \brief returns current message counter
  uint64_t currentMessageId() const;

  /// \brief serializes and sends to producer, returns bytes
  size_t produce();

  // \todo make private?
  /// \brief serializes buffer
  Buffer<uint8_t> serialize();

private:
  // \todo should this not be predefined in terms of jumbo frame?
  size_t max_events_{0};
  size_t events_{0};

  // \todo maybe should be mutated directly in buffer?
  uint64_t message_id_{1};

  ProducerCallback producer_callback_;

  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder builder;
  uint8_t *timeptr{nullptr};
  uint8_t *pixelptr{nullptr};

  EventMessage *eventMsg;

  Buffer<uint8_t> buffer;
  flatbuffers::uoffset_t *timeLenPtr;
  flatbuffers::uoffset_t *pixelLenPtr;
};
