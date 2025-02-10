// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ev44 flatbuffer schema serialiser. This serializer
/// is optimized around reduced memory allocation by rewriting the flatbuffer
/// builder's buffers in place. This should increase the performance and mem
/// copy.
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/serializer/AbstractSerializer.h>
#include <common/kafka/Producer.h>
#include <common/time/TSCTimer.h>
#include <flatbuffers/flatbuffers.h>

struct Event44Message;

/// \brief Statistics for EV44Serializer
struct EV44SerializerStats : public fbserializer::SerializerStats {
  int64_t ProduceTriggeredMaxEvents{0}; ///< Number of times the produce()
                                           ///< function has been called due to
                                           ///< the maximum number of events
                                           ///< being reached.
};

class EV44Serializer {
public:
  /// \brief creates ev44 flat buffer serializer
  /// \param max_array_length maximum number of events
  /// \param source_name value for source_name field
  EV44Serializer(size_t MaxArrayLength, const std::string &SourceName,
                 ProducerCallback Callback = {});

  virtual ~EV44Serializer() = default;

  /// \brief Explicitly disallow copy constructor
  EV44Serializer(const EV44Serializer &other) = delete;

  /// \brief sets producer callback
  /// \param cb function to be called to send buffer to Kafka
  void setProducerCallback(ProducerCallback Callback);

  /// \brief checks if new reference time being used, if so message needs to be
  /// produced
  uint32_t checkAndSetReferenceTime(int64_t Time);

  /// \brief changes reference time.
  /// Function is virtual to allow mocking
  virtual void setReferenceTime(int64_t Time);

  /// \returns the currently set reference time
  int64_t referenceTime() const;

  /// \brief adds event, if maximum count is exceeded, sends data using the
  /// producer callback \param Time time of event in relation to pulse time
  /// Function is virtual to allow mocking
  /// \param Pixel id of pixel as defined by logical geometry mapping
  /// \returns bytes transmitted, if any
  virtual size_t addEvent(int32_t Time, int32_t Pixel);

  /// \returns current message counter
  uint64_t currentMessageId() const;

  /// \brief returns event count
  size_t eventCount() const;

  /// \brief serializes and sends to producer
  /// \returns bytes transmitted
  size_t produce();

  // \todo make private?
  /// \brief serializes buffer
  /// \returns reference to internally stored buffer
  nonstd::span<const uint8_t> serialize();

  /// \brief returns statistics
  /// \returns reference to statistics object
  EV44SerializerStats &stats() { return Stats; }

  TSCTimer ProduceTimer, DebugTimer;

  // Counters for causes of calls to produce()
  int64_t ProduceCausePulseChange;
  int64_t ProduceCauseMaxEventsReached;

private:
  /// \todo should this not be predefined in terms of jumbo frame?
  size_t MaxEvents{0};
  size_t EventCount{0};

  uint64_t MessageId{1};

  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder Builder_;

  ProducerCallback ProduceFunctor;

  EV44SerializerStats Stats;

  uint8_t *ReferenceTimePtr{nullptr};
  uint8_t *OffsetTimePtr{nullptr};
  uint8_t *PixelPtr{nullptr};

  Event44Message *Event44Message_;

  nonstd::span<const uint8_t> Buffer_;
  flatbuffers::uoffset_t *TimeLengthPtr;
  flatbuffers::uoffset_t *PixelLengthPtr;
};
