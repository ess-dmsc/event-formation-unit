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

#include "span.hpp"
#include <common/Buffer.h>
#include <functional>
#include <memory>

///
class ProducerBase {
public:
  ProducerBase() = default;
  virtual ~ProducerBase() = default;

  /// \brief Send data to Kafka broker on previously set topic.
  /// Will accept a buffer of (almost) arbitrary type. See unit tests for
  /// examples.
  /// \param Buffer Reference to a buffer
  /// \param MessageTimestampMS Timestamp of message in milliseconds since UNIX
  /// epoch
  /// \return Returns 0 on success, another value on failure.
  virtual int produce(nonstd::span<const std::uint8_t> Buffer,
                      std::int64_t MessageTimestampMS) = 0;
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

  int produce(nonstd::span<const std::uint8_t> Buffer,
              std::int64_t MessageTimestampMS) override;

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

protected:
  std::string ErrorMessage;
  std::string TopicName;
  std::unique_ptr<RdKafka::Conf> Config;
  std::unique_ptr<RdKafka::Conf> TopicConfig;
  std::unique_ptr<RdKafka::Topic> KafkaTopic;
  std::unique_ptr<RdKafka::Producer> KafkaProducer;
};

using ProducerCallback = std::function<void(nonstd::span<const std::uint8_t>, std::int64_t)>;
