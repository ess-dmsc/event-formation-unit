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
};

using Queue = moodycamel::ReaderWriterQueue<std::unique_ptr<EventData>>;

class EventSerializer {
public:
  EventSerializer(std::string SourceName, size_t BufferSize,
                  std::chrono::milliseconds TransmitTimeout,
                  ProducerBase *KafkaProducer);
  virtual ~EventSerializer();
  void addEvent(std::unique_ptr<EventData> Event);
  void setTransmitTimeout(std::chrono::milliseconds TransmitTimeout) {
    Timeout = TransmitTimeout;
  }

private:
  void serialiseFunction();
  bool RunThread{true};
  std::string Name;
  std::chrono::milliseconds Timeout;
  size_t EventBufferSize;
  ProducerBase *Producer;
  std::thread SerializeThread;
  Queue EventQueue;
};
