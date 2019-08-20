//
// Created by Jonas Nilsson on 2019-07-09.
//

#pragma once

#include <chrono>
#include <common/Producer.h>
#include <memory>
#include <readerwriterqueue/readerwriterqueue.h>
#include <string>
#include <thread>

struct EventData {
  std::uint64_t Timestamp{0};
  std::uint32_t EventId{0};
  std::uint32_t Amplitude{0};
  std::uint32_t PeakArea{0};
  std::uint32_t Background{0};
  std::uint64_t ThresholdTime{0};
  std::uint64_t PeakTime{0};
  std::uint64_t minTimestamp() {
    std::uint64_t ReturnValue = Timestamp;
    if (ThresholdTime != 0 and ThresholdTime < ReturnValue) {
      ReturnValue = ThresholdTime;
    }
    if (PeakTime != 0 and PeakTime < ReturnValue) {
      ReturnValue = PeakTime;
    }
    return ReturnValue;
  };
  std::uint64_t maxTimestamp() {
    std::uint64_t ReturnValue = Timestamp;
    if (ThresholdTime != 0 and ThresholdTime > ReturnValue) {
      ReturnValue = ThresholdTime;
    }
    if (PeakTime != 0 and PeakTime > ReturnValue) {
      ReturnValue = PeakTime;
    }
    return ReturnValue;
  };
};

/// \brief Simple event buffer that keeps track of events based on their timestamps
class EventBuffer {
public:
  /// \brief Initializes the event buffer.
  ///
  /// \param BufferSize The maximum number events in the buffer.
  EventBuffer(size_t BufferSize);
  /// \brief Used for setting the reference timestamps for determining if the buffer should be culled/emptied.
  ///
  /// \param ReferenceTime Reference timestamp.
  /// \param Timespan
  void setReferenceTimes(std::uint64_t ReferenceTime, std::uint64_t Timespan);
  void addEvent(std::unique_ptr<EventData[]> Event);
  bool shouldCullEvents();
  nonstd::span<EventData> getEvents(bool AllEvents);
  void clearEvents(bool AllEvents);
private:
  std::unique_ptr<EventData[]> Events;
  std::uint64_t RefTime;
  std::uint64_t MaxOffsetTime;
};

using Queue = moodycamel::ReaderWriterQueue<std::unique_ptr<EventData>>;
using TimestampQueue = moodycamel::ReaderWriterQueue<std::uint64_t>;

/// \brief Serializes events from a data source (event formation kernel).
/// Transforms the timestamps of the events based on which mode it is operating in.
class EventSerializer {
public:
  enum class TimestampMode {
    INDEPENDENT_EVENTS,
    TIME_REFERENCED,
  };
  /// \brief Constructor that handles the set-up of the serializer. Also launches the thread that does the actual
  /// serialization.
  /// \param[in] SourceName Used as the source name in the flatbuffer that is produced.
  /// \param[in] BufferSize The maximum number of events that is stored in the flatbuffer.
  /// \param[in] TransmitTimeout If no event has been received in this many milliseconds (provided by the operating
  /// system), transmit (produce) the flatbuffer anyway.
  /// \note The size (in bytes) of the flatbuffer that is produced will be the same regardless of the number of events
  /// that it actually contains.
  /// \param[in] KafkaProducer The Kafka producer that is used when a flatbuffer is done.
  /// \param[in] Mode The timestamp mode in which the serializer is operating. The different modes have the following
  /// effects:
  ///  - INDEPENDENT_EVENTS
  ///    - Use the first timestamp in a set of events as the reference timestamp.
  ///    - EventSerializer::addReferenceTimestamp() does nothing.
  ///    - A flatbuffer is produced when either:
  ///      - It is full.
  ///      - TransmitTimeout has passed since the first event.
  ///      - The difference between the reference timestamp and the current event timestamp is greater than
  ///        std::numeric_limits<std::uint32_t>::max().
  ///  - TIME_REFERENCED
  ///    - Before the first call to EventSerializer::addReferenceTimestamp(), buffer BufferSize events and use the
  ///      timestamp of the first event as the reference time.
  ///    - Use the timestamp from EventSerializer::addReferenceTimestamp() as a reference timestamp.
  ///    - If the time difference between the last reference timestamp and the event timestamp is greater than
  ///      std::numeric_limits<std::uint32_t>::max(), use the event timestamp as the next reference.
  ///    - A flatbuffer is produced when either:
  ///      - It is full.
  ///      - TransmitTimeout has passed since the first event.
  ///      - The difference between the reference timestamp and the current event timestamp is greater than
  ///        std::numeric_limits<std::uint32_t>::max().
  ///      - There is a new reference timestamp that is smaller than the last buffered or currently processed
  ///        event timestamp.
  EventSerializer(std::string SourceName, size_t BufferSize,
                  std::chrono::milliseconds TransmitTimeout,
                  ProducerBase *KafkaProducer, TimestampMode Mode);
  /// \brief Stops the thread and may for this reason take a long time before returning.
  virtual ~EventSerializer();
  void addReferenceTimestamp(std::uint64_t const Timestamp);
  /// \brief Add new event to queue of events that will be processed.
  ///
  /// \param Event Struct of event data.
  /// \note EventData::ThresholdTime and EventData::PeakTime must be set to 0 if not in use.
  /// \note EventData::Timestamp, EventData::ThresholdTime and EventData::PeakTime must all be within
  /// std::numeric_limits<std::uint32_t>::max() of each other if not set to zero.
  void addEvent(std::unique_ptr<EventData> Event);
  void setTransmitTimeout(std::chrono::milliseconds TransmitTimeout) {
    Timeout = TransmitTimeout;
  }
protected:
  virtual std::chrono::system_clock::time_point getCurrentTime() const {
    return std::chrono::system_clock::now();
  };
  void serialiseFunction();
  bool RunThread{true};
  std::string Name;
  std::chrono::milliseconds Timeout;
  size_t EventBufferSize;
  ProducerBase *Producer;
  std::thread SerializeThread;
  Queue EventQueue;
  TimestampQueue ReferenceTimeQueue;
  const TimestampMode CMode;
};
