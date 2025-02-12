// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for sending data to Kafka broker
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <cstdint>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <librdkafka/rdkafkacpp.h>
#pragma GCC diagnostic pop

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/memory/Buffer.h>
#include <common/memory/span.hpp>
#include <functional>
#include <memory>
#include <unordered_map> // add if not already included
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
  virtual int produce(const nonstd::span<const std::uint8_t> &Buffer,
                      const std::int64_t MessageTimestampMS) = 0;
};

/// \brief The Producer class is responsible for producing Kafka messages and
/// sending them to the cluster.
///
/// It inherits from ProducerBase and RdKafka::EventCb.
class Producer : public ProducerBase,
                 public RdKafka::EventCb,
                 public RdKafka::DeliveryReportCb {
public:
  /// \brief Constructs a Producer object.
  ///
  /// \param Broker The 'URL' specifying the host and port, for example,
  /// "127.0.0.1:9009".
  /// \param Topic The name of the Kafka topic according to the agreement, for
  /// example, "trex_detector".
  /// \param Configs A vector of configuration <type,value> pairs.
  Producer(const std::string &Broker, const std::string &Topic,
           std::vector<std::pair<std::string, std::string>> &Configs,
           Statistics *Stats = nullptr);

  /// \brief Cleans up by deleting allocated structures.
  ~Producer() = default;

  /// \brief Structure to hold producer statistics.
  struct ProducerStats {
    /// \brief Count of configuration errors
    int64_t config_errors{0};
    /// \brief Count of bytes successfully produced
    int64_t produce_bytes_ok{0};
    /// \brief Count of bytes that failed to produce
    int64_t produce_bytes_error{0};
    /// \brief Total number of produce() calls
    int64_t produce_calls{0};
    /// \brief Count of failed produce() calls
    int64_t produce_errors{0};

    // librdkafka error statistics
    /// \brief Count of timeout errors
    int64_t ErrTimeout{0};
    /// \brief Count of transport errors
    int64_t ErrTransport{0};
    /// \brief Count of broker not available errors
    int64_t ErrBrokerNotAvailable{0};
    /// \brief Count of unknown topic errors
    int64_t ErrTopic{0};
    /// \brief Count of queue full errors
    int64_t ErrQueueFull{0};
    /// \brief Count of other/unspecified errors
    int64_t ErrOther{0};
    /// \brief Count of message timeout errors
    int64_t ErrMsgTimeout{0};
    /// \brief Count of kafka authentication errors
    int64_t ErrAuth{0};
    /// \brief Count of message size too large errors
    int64_t ErrMsgSizeTooLarge{0};
    /// \brief Count of unknown partition errors
    int64_t ErrUknownPartition{0};
    /// \brief Total count of transmission errors
    int64_t TransmissionErrors{0};
    /// \brief Count of delivery reports with errors
    int64_t MsgError{0};

    // librdkafka message statistics
    /// \brief Count of successful message deliveries
    int64_t MsgDeliverySuccess{0};
    /// \brief Count of messages confirmed as persisted
    int64_t MsgStatusPersisted{0};
    /// \brief Count of messages possibly persisted but unconfirmed
    int64_t MsgStatusPossiblyPersisted{0};
    /// \brief Count of messages confirmed as not persisted
    int64_t MsgStatusNotPersisted{0};
    /// \brief Total count of delivery report events
    int64_t TotalMsgDeliveryEvent{0};
    /// \brief Current number of messages in queue
    int64_t NumberOfMsgInQueue{0};
    /// \brief Maximum number of messages in queue
    int64_t MaxNumOfMsgInQueue{0};
    /// \brief Current bytes of messages in queue
    int64_t BytesOfMsgInQueue{0};
    /// \brief Maximum bytes of messages in queue
    int64_t MaxBytesOfMsgInQueue{0};

    // librdkafka transmission statistics
    /// \brief Total bytes transmitted to brokers
    int64_t BytesTransmittedToBrokers{0};
    /// \brief Count of transmission request retries
    int64_t TxRequestRetries{0};
  } __attribute__((aligned(64)));

  /// \brief Polls the producer for events.
  void poll(int TimeoutMS) { KafkaProducer->poll(TimeoutMS); };

  void dr_cb(RdKafka::Message &message) override;

  void event_cb(RdKafka::Event &event) override;

  /// \brief Returns the producer statistics reference.
  /// \return ProducerStats The producer statistics.
  const ProducerStats &getStats() const { return ProducerStatCounters; }

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

  std::string ErrorMessage;
  std::string TopicName;
  std::unique_ptr<RdKafka::Conf> Config;
  std::unique_ptr<RdKafka::Conf> TopicConfig;
  std::unique_ptr<RdKafka::Topic> KafkaTopic;
  std::unique_ptr<RdKafka::Producer> KafkaProducer;

private:
  ProducerStats ProducerStatCounters = {};

  inline void applyKafkaErrorCode(RdKafka::ErrorCode ErrorCode) noexcept {

    // First log the error and its error string.
    LOG(KAFKA, Sev::Warning, "Rdkafka::Event::EVENT_ERROR [{}]: {}",
        static_cast<int>(ErrorCode), RdKafka::err2str(ErrorCode).c_str());
    XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR [%d]: %s\n", ErrorCode,
           RdKafka::err2str(ErrorCode).c_str());

    switch (ErrorCode) {
    case RdKafka::ErrorCode::ERR__TIMED_OUT:
      ++ProducerStatCounters.ErrTimeout;
      break;
    case RdKafka::ErrorCode::ERR__TRANSPORT:
      ++ProducerStatCounters.ErrTransport;
      break;
    case RdKafka::ErrorCode::ERR_BROKER_NOT_AVAILABLE:
      ++ProducerStatCounters.ErrBrokerNotAvailable;
      break;
    case RdKafka::ErrorCode::ERR__UNKNOWN_TOPIC:
      ++ProducerStatCounters.ErrTopic;
      break;
    case RdKafka::ErrorCode::ERR_TOPIC_EXCEPTION:
      ++ProducerStatCounters.ErrTopic;
      break;
    case RdKafka::ErrorCode::ERR_TOPIC_AUTHORIZATION_FAILED:
      ++ProducerStatCounters.ErrTopic;
      break;
    case RdKafka::ErrorCode::ERR__QUEUE_FULL:
      ++ProducerStatCounters.ErrQueueFull;
      break;
    case RdKafka::ErrorCode::ERR__MSG_TIMED_OUT:
      ++ProducerStatCounters.ErrMsgTimeout;
      break;
    case RdKafka::ErrorCode::ERR__AUTHENTICATION:
      ++ProducerStatCounters.ErrAuth;
      break;
    case RdKafka::ErrorCode::ERR_MSG_SIZE_TOO_LARGE:
      ++ProducerStatCounters.ErrMsgSizeTooLarge;
      break;
    case RdKafka::ErrorCode::ERR__UNKNOWN_PARTITION:
      ++ProducerStatCounters.ErrUknownPartition;
      break;
    case RdKafka::ErrorCode::ERR_NO_ERROR:
      break;
    default:
      ++ProducerStatCounters.ErrOther;
      break;
    }
  }
};

using ProducerCallback =
    std::function<void(const nonstd::span<const std::uint8_t> &, std::int64_t)>;
