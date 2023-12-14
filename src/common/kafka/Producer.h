// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
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

#include <common/memory/Buffer.h>
#include <functional>
#include <memory>
#include <common/memory/span.hpp>
#include <utility>
#include <vector>

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
  /// \param Broker 'URL' specifying host and port, example "127.0.0.1:9009"
  /// \param Topic Name of Kafka topic according to agreement, example
  /// "trex_detector"
  /// \param Configs vector of configuration <type,value> pairs
  Producer(std::string Broker, std::string Topic,
           std::vector<std::pair<std::string, std::string>> &Configs);

  /// \brief cleans up by deleting allocated structures
  ~Producer() = default;

  ///\brief Produce kafka messages and send to cluster
  ///\return int, 0 if successful
  int produce(nonstd::span<const std::uint8_t> Buffer,
              std::int64_t MessageTimestampMS) override;

  /// \brief set kafka configuration and check result
  RdKafka::Conf::ConfResult setConfig(std::string Key, std::string Value);

  /// \brief Kafka callback function for events
  void event_cb(RdKafka::Event &event) override;

  struct ProducerStats {
    int64_t config_errors;
    int64_t ev_errors;
    int64_t ev_others;
    int64_t dr_errors;
    int64_t dr_noerrors;
    int64_t produce_calls;
    int64_t produce_fails;
    int64_t produce_no_errors;
    int64_t err_unknown_topic;
    int64_t err_queue_full;
    int64_t err_other;
    int64_t librdkafka_msg_cnt;
    int64_t librdkafka_msg_size;
  } stats = {};

protected:
  std::string ErrorMessage;
  std::string TopicName;
  std::unique_ptr<RdKafka::Conf> Config;
  std::unique_ptr<RdKafka::Conf> TopicConfig;
  std::unique_ptr<RdKafka::Topic> KafkaTopic;
  std::unique_ptr<RdKafka::Producer> KafkaProducer;
};

using ProducerCallback =
    std::function<void(nonstd::span<const std::uint8_t>, std::int64_t)>;
