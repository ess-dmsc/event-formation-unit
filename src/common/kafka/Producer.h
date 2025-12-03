// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for sending data to Kafka broker
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/StatCounterBase.h>
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
#include <malloc.h>
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
  virtual int produce(const nonstd::span<const uint8_t> &Buffer,
                      int64_t MessageTimestampMS) = 0;
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
  /// \param Stats Reference to Statistics object for counter registration.
  /// \param Name Name of the producer instance for statistics prefix.
  Producer(const std::string &Broker, const std::string &Topic,
           std::vector<std::pair<std::string, std::string>> &Configs,
           Statistics &Stats, const std::string &Name = "event");

  /// \brief Cleans up by deleting allocated structures.
  ~Producer() = default;

  /// \brief Structure to hold producer statistics.
  struct ProducerStats : public StatCounterBase {
    /// \brief Count of bytes successfully produced
    int64_t ProduceBytesOk{0};
    /// \brief Count of bytes that failed to produce
    int64_t ProduceBytesError{0};
    /// \brief Total number of produce() calls
    int64_t ProduceCalls{0};
    /// \brief Count of failed produce() calls
    int64_t ProduceError{0};
    /// \brief Count of malloc_trim calls made
    int64_t MallocTrimCalls{0};

    // librdkafka errors
    /// \brief Count errors during librdkafka config operations
    int64_t ErrConfig{0};
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
    /// \brief Total count of statistics events
    int64_t StatsEventCounter{0};
    /// \brief Total count of error events
    int64_t ErrorEventCounter{0};

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
    int64_t BytesInQueue{0};
    /// \brief Maximum bytes of messages in queue
    int64_t MaxBytesInQueue{0};

    // librdkafka transmission statistics
    /// \brief Total bytes transmitted to brokers
    int64_t BytesTransmittedToBrokers{0};
    /// \brief Count of transmission request retries
    int64_t TxRequestRetries{0};

    /// \brief Constructor that registers all counters with Statistics
    ProducerStats(Statistics &Stats, const std::string &Prefix)
        : StatCounterBase(
              Stats,
              {{"stat_events", StatsEventCounter},
               {"error_events", ErrorEventCounter},
               {"produce_bytes_ok", ProduceBytesOk},
               {"produce_bytes_error", ProduceBytesError},
               {"produce_calls", ProduceCalls},
               {"produce_errors", ProduceError},
               
              /// performance indicators
               {"memory_cleanups", MallocTrimCalls},
               
               /// librdkafka transmission stats
               {"brokers.tx_bytes", BytesTransmittedToBrokers},
               {"brokers.tx_req_retries", TxRequestRetries},

               /// librdkafka message stats
               {"msg.num_of_msg_in_queue", NumberOfMsgInQueue},
               {"msg.max_num_of_msg_in_queue", MaxNumOfMsgInQueue},
               {"msg.bytes_in_queue", BytesInQueue},
               {"msg.max_bytes_in_queue", MaxBytesInQueue},
               {"msg.delivery_success", MsgDeliverySuccess},
               {"msg.status_persisted", MsgStatusPersisted},
               {"msg.status_not_persisted", MsgStatusNotPersisted},
               {"msg.status_possibly_persisted", MsgStatusPossiblyPersisted},
               {"msg.msg_delivery_event", TotalMsgDeliveryEvent},

               /// librdkafka error stats
               {"error.msg_delivery", MsgError},
               {"error.config", ErrConfig},
               {"error.transmission", TransmissionErrors},
               {"error.unknown_topic", ErrTopic},
               {"error.unknown_partition", ErrUknownPartition},
               {"error.queue_full", ErrQueueFull},
               {"error.timeout", ErrTimeout},
               {"error.msg_timeout", ErrMsgTimeout},
               {"error.transport", ErrTransport},
               {"error.authentication", ErrAuth},
               {"error.msg_size", ErrMsgSizeTooLarge},
               {"error.other", ErrOther}},
              Prefix) {}
  } __attribute__((aligned(64)));

  /// \brief Polls the producer for events and checks queue length.
  /// and triggers memory recovery if needed.
  /// \param TimeoutMS The timeout in milliseconds for polling.
  inline void poll(int TimeoutMS) {

    int current = KafkaProducer->outq_len();

    // Track high watermark occurrence
    HighWatermarkReached = current >= QueueHighWatermark;

    // If queue dropped significantly from peak (>50% to <1%), trigger cleanup
    if (HighWatermarkReached && current < QueueRecoveryThreshold) {

      malloc_trim(0);
      StatCounters.MallocTrimCalls++;
      HighWatermarkReached = false;
      XTRACE(KAFKA, INF,
             "Kafka producer queue recovered, triggered malloc_trim");
    }

    if (!KafkaProducer)
      return;
    KafkaProducer->poll(TimeoutMS);
    StatCounters.NumberOfMsgInQueue = KafkaProducer->outq_len();
  };

  /// \brief Delivery report callback. This function is called when we
  /// processing delivery reports from the producer when calling poll().
  void dr_cb(RdKafka::Message &message) override;

  /// \brief Event callback. This function is called when we receive events from
  /// the producer when calling poll().
  void event_cb(RdKafka::Event &event) override;

  /// \brief Returns the producer statistics reference.
  /// \return ProducerStats The producer statistics.
  inline const ProducerStats &getStats() const { return StatCounters; }

  /// \brief Produces Kafka messages and sends them to the cluster and increment
  /// internal counters. This function is non-blocking, returns immediately
  /// after the message is enqueued for transmission.
  ///
  /// \param Buffer The buffer containing the message data.
  /// \param MessageTimestampMS The timestamp of the message in milliseconds.
  /// \return int Returns 0 if the operation is successful or an error code
  int produce(const nonstd::span<const uint8_t> &Buffer,
              int64_t MessageTimestampMS) override;

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
  /// Local struct to store producer statistics. References of these counters
  /// are registered into the Statistics object.
  ProducerStats StatCounters;

  /// \brief Calculated based on the configured max queue size.
  int QueueRecoveryThreshold{0};
  int QueueHighWatermark{0};

  bool HighWatermarkReached{false};

  void applyKafkaErrorCode(RdKafka::ErrorCode ErrorCode);
};

using ProducerCallback =
    std::function<void(const nonstd::span<const uint8_t> &, int64_t)>;
