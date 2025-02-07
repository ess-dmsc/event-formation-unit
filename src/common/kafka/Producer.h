// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for sending data to Kafka broker
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <librdkafka/rdkafkacpp.h>
#pragma GCC diagnostic pop

#include <common/memory/Buffer.h>
#include <common/memory/span.hpp>
#include <functional>
#include <memory>
#include <unordered_map> // add if not already included
#include <utility>
#include <vector>

// New handler classes
class KafkaEventHandler : public RdKafka::EventCb {
public:
  KafkaEventHandler() {}

  void event_cb(RdKafka::Event &event) override;

  uint64_t ErrTimeout{0};
  uint64_t ErrTransport{0};
  uint64_t ErrBrokerNotAvailable{0};
  uint64_t ErrUnknownTopic{0};
  uint64_t ErrQueueFull{0};
  uint64_t ErrOther{0};

  // Counters for statistics
  int64_t NumberOfMessageInQueue{0};
  int64_t SizeOfMessageInQueue{0};
  int64_t BytesTransmittedToBrokers{0};
  int64_t TransmissionErrors{0};
  int64_t TxRequestRetries{0};
};

class DeliveryReportHandler : public RdKafka::DeliveryReportCb {
public:
  DeliveryReportHandler() : totalCount(0), errorCount(0), successCount(0) {}
  void dr_cb(RdKafka::Message &message) override;
  uint64_t totalCount;   // Total delivery reports received
  uint64_t errorCount;   // Count of delivery reports with error
  uint64_t successCount; // Count of successful delivery reports
};

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
  virtual int produce(const nonstd::span<const std::uint8_t> &Buffer,
                      const std::int64_t MessageTimestampMS) = 0;
};

/// \brief The Producer class is responsible for producing Kafka messages and
/// sending them to the cluster.
///
/// It inherits from ProducerBase and RdKafka::EventCb.
class Producer : public ProducerBase {
public:
  /// \brief Constructs a Producer object.
  ///
  /// \param Broker The 'URL' specifying the host and port, for example,
  /// "127.0.0.1:9009".
  /// \param Topic The name of the Kafka topic according to the agreement, for
  /// example, "trex_detector".
  /// \param Configs A vector of configuration <type,value> pairs.
  Producer(const std::string &Broker, const std::string &Topic,
           std::vector<std::pair<std::string, std::string>> &Configs);

  /// \brief Cleans up by deleting allocated structures.
  ~Producer() = default;

  /// \brief Produces Kafka messages and sends them to the cluster and increment
  /// internal counters. This function is non-blocking, returns immediately
  /// after the message is enqueueed for transmission.
  ///
  /// \param Buffer The buffer containing the message data.
  /// \param MessageTimestampMS The timestamp of the message in milliseconds.
  /// \return int Returns 0 if the operation is successful or an error code
  int produce(const nonstd::span<const std::uint8_t> &Buffer,
              std::int64_t MessageTimestampMS) override;

  /// \brief Sets a Kafka configuration and checks the result.
  ///
  /// \param Key The configuration key.
  /// \param Value The configuration value.
  /// \return RdKafka::Conf::ConfResult The result of setting the configuration.
  RdKafka::Conf::ConfResult setConfig(const std::string &Key,
                                      const std::string &Value);

  /// \brief Structure to hold producer statistics.
  struct ProducerStats {
    int64_t config_errors;
    int64_t ev_stats;
    int64_t ev_logs;
    int64_t ev_throttle;
    int64_t ev_errors;
    int64_t ev_others;
    int64_t dr_errors;
    int64_t dr_noerrors;
    int64_t produce_bytes_ok;
    int64_t produce_bytes_error;
    int64_t produce_calls;
    int64_t produce_errors;
    int64_t produce_no_errors;
    int64_t err_timeout;
    int64_t err_transport;
    int64_t err_unknown_topic;
    int64_t err_queue_full;
    int64_t err_other;
    int64_t librdkafka_msg_cnt;
    int64_t librdkafka_msg_size;
    int64_t librdkafka_tx_bytes;
    int64_t librdkafka_brokers_waitresp = 0;
    int64_t librdkafka_brokers_tx_byte = 0;
    int64_t librdkafka_brokers_txerrors = 0;
  } stats = {};

  KafkaEventHandler EventHandler;
  DeliveryReportHandler DeliveryHandler;

protected:
  std::string ErrorMessage;
  std::string TopicName;
  std::unique_ptr<RdKafka::Conf> Config;
  std::unique_ptr<RdKafka::Conf> TopicConfig;
  std::unique_ptr<RdKafka::Topic> KafkaTopic;
  std::unique_ptr<RdKafka::Producer> KafkaProducer;
};

using ProducerCallback =
    std::function<void(const nonstd::span<const std::uint8_t> &, std::int64_t)>;
