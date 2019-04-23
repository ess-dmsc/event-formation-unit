/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for sending data to Kafka broker
///
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <librdkafka/rdkafkacpp.h>
#pragma GCC diagnostic pop

#include <common/Buffer.h>
#include <functional>
#include "span.hpp"
#include <memory>

///
class ProducerBase {
public:
  ProducerBase() = default;
  virtual ~ProducerBase() = default;

  // \todo deprecate this function in favor of encapsulated buffer
  [[deprecated("Due to problematic use of system time.")]]
  virtual int produce(void* buffer, size_t bytes) = 0;
  
  template<typename T>
  [[deprecated("Due to problematic use of system time.")]]
  inline void produce2(const Buffer<T> &buffer)
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
    this->produce(buffer.address, buffer.bytes());
#pragma GCC diagnostic pop
  }
  
  
  virtual int produce(const nonstd::span<const std::uint8_t> &Buffer, std::int64_t MessageTimestampMS) = 0;
  
  /// \brief Send data to Kafka broker on previously set topic.
  /// \param Buffer Reference to a buffer
  template<typename T>
  int produce(const nonstd::span<const T> &Buffer, std::int64_t MessageTimestampMS)
  {
    return produce({Buffer.data(), Buffer.size_bytes()}, MessageTimestampMS);
  }
};

class Producer : public ProducerBase, public RdKafka::EventCb {
public:
  /// \brief Construct a producer object.
  /// \param broker 'URL' specifying host and port, example "127.0.0.1:9009"
  /// \param topicstr Name of Kafka topic according to agreement, example
  /// "T-REX_detectors"
  Producer(std::string Broker, std::string topicstr);

  /// \brief cleans up by deleting allocated structures
  ~Producer() = default;

  /** \brief Function called to send data to a broker
   *  @param buffer Pointer to char buffer containing data to be tx'ed
   *  @param length Size of buffer data in bytes
   */
  [[deprecated("Due to problematic use of system time.")]]
  int produce(void* buffer, size_t bytes) override {
    return produce({reinterpret_cast<const std::uint8_t*>(buffer), static_cast<ssize_t>(bytes)}, time(nullptr) * 1000l);
  };
  
  int produce(const nonstd::span<const std::uint8_t> &Buffer, std::int64_t MessageTimestampMS) override;

  /// \brief set kafka configuration and check result
  void setConfig(std::string Key, std::string Value);

  /// \brief Kafka callback function for events
  void event_cb(RdKafka::Event &event) override;

  struct {
    uint64_t ev_errors;
    uint64_t ev_others;
    // uint64_t ev_log;
    // uint64_t ev_stats;
    // uint64_t ev_throttle;
    uint64_t dr_errors;
    uint64_t dr_noerrors;
    uint64_t produce_fails;
  } stats = {};

private:
  std::string ErrorMessage;
  std::string TopicName;
  std::unique_ptr<RdKafka::Conf> Config;
  std::unique_ptr<RdKafka::Conf> TopicConfig;
  std::unique_ptr<RdKafka::Topic> KafkaTopic;
  std::unique_ptr<RdKafka::Producer> KafkaProducer;
};

using ProducerCallback = std::function<void(Buffer<uint8_t>)>;
