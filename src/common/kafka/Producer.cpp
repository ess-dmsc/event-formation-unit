// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Kafka producer - wrapper for librdkafka
///
/// \note: See https://github.com/edenhill/librdkafka
///
/// For more information the avaialbe producer statistics see:
/// https://github.com/confluentinc/librdkafka/blob/master/STATISTICS.md
///
//===----------------------------------------------------------------------===//

#include <cassert>
#include <common/kafka/Producer.h>
#include <common/system/gccintel.h>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

RdKafka::Conf::ConfResult Producer::setConfig(const std::string &Key,
                                              const std::string &Value) {
  // Don't log passwords
  std::string LogValue{Value};
  if (Key == "sasl.password") {
    LogValue = "<REDACTED>";
  }

  XTRACE(INIT, ALW, "%s %s", Key.c_str(), LogValue.c_str());
  RdKafka::Conf::ConfResult configResult;
  configResult = Config->set(Key, Value, ErrorMessage);
  LOG(KAFKA, Sev::Info, "Kafka set config {} to {}", Key, LogValue);
  if (configResult != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka Unable to set config {} to {}", Key,
        LogValue);
    ProducerStats.config_errors++;
  }
  return configResult;
}

///
Producer::Producer(const std::string &Broker, const std::string &Topic,
                   std::vector<std::pair<std::string, std::string>> &Configs,
                   Statistics *Stats)
    : ProducerBase(), TopicName(Topic) {

  /// If Stats is nullptr, internal stat counters will not be registered
  if (Stats != nullptr) {
    // clang-format off
    Stats->create("kafka.config_errors", ProducerStats.config_errors);
    Stats->create("kafka.stat_events", ProducerStats.StatsEventCounter);
    Stats->create("kafka.produce_bytes_ok", ProducerStats.produce_bytes_ok);
    Stats->create("kafka.produce_bytes_error", ProducerStats.produce_bytes_error);
    Stats->create("kafka.produce_calls", ProducerStats.produce_calls);
    Stats->create("kafka.produce_errors", ProducerStats.produce_errors);
    
    /// librdkafka transmission stats
    Stats->create("kafka.brokers.tx_bytes", ProducerStats.BytesTransmittedToBrokers);
    Stats->create("kafka.brokers.tx_req_retries", ProducerStats.TxRequestRetries);
    
    /// librdkafka message stats
    Stats->create("kafka.msg.num_of_msg_in_queue", ProducerStats.NumberOfMsgInQueue);
    Stats->create("kafka.msg.max_num_of_msg_in_queue", ProducerStats.MaxNumOfMsgInQueue);
    Stats->create("kafka.msg.bytes_of_msg_in_queue", ProducerStats.BytesOfMsgInQueue);
    Stats->create("kafka.msg.max_bytes_of_msg_in_queue", ProducerStats.MaxBytesOfMsgInQueue);
    Stats->create("kafka.msg.delivery_success", ProducerStats.MsgDeliverySuccess);
    Stats->create("kafka.msg.status_persisted", ProducerStats.MsgStatusPersisted);
    Stats->create("kafka.msg.status_not_persisted", ProducerStats.MsgStatusNotPersisted);
    Stats->create("kafka.msg.status_possibly_persisted", ProducerStats.MsgStatusPossiblyPersisted);
    Stats->create("kafka.msg.msg_delivery_event", ProducerStats.TotalMsgDeliveryEvent);
    
    /// librdkafka error stats
    Stats->create("kafka.error.msg_delivery", ProducerStats.MsgError);
    Stats->create("kafka.error.transmission", ProducerStats.TransmissionErrors);
    Stats->create("kafka.error.unknown_topic", ProducerStats.ErrTopic);
    Stats->create("kafka.error.unknown_partition", ProducerStats.ErrUknownPartition);
    Stats->create("kafka.error.queue_full", ProducerStats.ErrQueueFull);
    Stats->create("kafka.error.timeout", ProducerStats.ErrTimeout);
    Stats->create("kafka.error.msg_timeout", ProducerStats.ErrMsgTimeout);
    Stats->create("kafka.error.transport", ProducerStats.ErrTransport);
    Stats->create("kafka.error.authentication", ProducerStats.ErrAuth);
    Stats->create("kafka.error.msg_size", ProducerStats.ErrMsgSizeTooLarge);
    Stats->create("kafka.error.other", ProducerStats.ErrOther);
    // clang-format on
  }

  /// Perform the configuration of the Kafka producer
  Config.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  TopicConfig.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  if (Config == nullptr) {
    LOG(KAFKA, Sev::Error, "Unable to create CONF_GLOBAL object");
    return;
  }

  if (TopicConfig == nullptr) {
    LOG(KAFKA, Sev::Error, "Unable to create CONF_TOPIC object");
    return;
  }

  setConfig("metadata.broker.list", Broker); // can be overwritten

  for (auto &Config : Configs) {
    setConfig(Config.first, Config.second);
  }

  /// Register this producer (this object) as the event call back handler to
  /// receive events
  if (Config->set("event_cb", static_cast<RdKafka::EventCb *>(this),
                  ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set event_cb");
  }

  /// Register this producer (this object) as the delivery report call back
  /// handler to receive delivery reports
  if (Config->set("dr_cb", static_cast<RdKafka::DeliveryReportCb *>(this),
                  ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set dr_cb");
  }

  // Set message timeout to 1 minute
  Config->set("message.timeout.ms", "60000", ErrorMessage);

  KafkaProducer.reset(RdKafka::Producer::create(Config.get(), ErrorMessage));
  if (!KafkaProducer) {
    LOG(KAFKA, Sev::Error, "Failed to create producer: {}", ErrorMessage);
    return;
  }

  KafkaTopic.reset(RdKafka::Topic::create(KafkaProducer.get(), TopicName,
                                          TopicConfig.get(), ErrorMessage));
  if (!KafkaTopic) {
    LOG(KAFKA, Sev::Error, "Failed to create topic: {}", ErrorMessage);
    return;
  }
}

int Producer::produce(const nonstd::span<const std::uint8_t> &Buffer,
                      std::int64_t MessageTimestampMS) {

  if (KafkaProducer == nullptr || KafkaTopic == nullptr) {
    return RdKafka::ERR_UNKNOWN;
  }

  // non-blocking, copies the buffer to kafka thread for transfer
  auto error = KafkaProducer->produce(
      TopicName, -1, RdKafka::Producer::RK_MSG_COPY,
      const_cast<std::uint8_t *>(Buffer.data()), Buffer.size_bytes(), NULL, 0,
      MessageTimestampMS, NULL);

  KafkaProducer->poll(0);

  ProducerStats.produce_calls++;

  if (error != RdKafka::ERR_NO_ERROR) {
    ProducerStats.produce_errors++;
    ProducerStats.produce_bytes_error += Buffer.size_bytes();
    applyKafkaErrorCode(error);

    LOG(KAFKA, Sev::Error, "Failed to produce message: {}",
        RdKafka::err2str(error));
    return error;
  }

  ProducerStats.produce_bytes_ok += Buffer.size_bytes();

  return 0;
}

// Implementation of KafkaEventHandler override
void Producer::event_cb(RdKafka::Event &event) {
  nlohmann::json res;
  ++ProducerStats.StatsEventCounter;

  /// initialize variable to sum up the values later when looping over brokers
  int64_t TransmissionErrors = 0;
  int64_t BytesTransmittedToBrokers = 0;
  int64_t TxRequestRetries = 0;

  switch (event.type()) {
  case RdKafka::Event::EVENT_STATS:
    res = nlohmann::json::parse(event.str());
    ProducerStats.NumberOfMsgInQueue = res["msg_cnt"].get<int64_t>();
    ProducerStats.MaxNumOfMsgInQueue = res["msg_max"].get<int64_t>();
    ProducerStats.BytesOfMsgInQueue = res["msg_size"].get<int64_t>();
    ProducerStats.MaxBytesOfMsgInQueue = res["msg_size_max"].get<int64_t>();

    /// \note loop over brokers and sum up the values into local variables
    /// before assigning them to the ProducerStats struct.
    /// This is to impriove thread safety becase the ProducerStats struct
    /// is registered into the Statistics object and can be accessed from
    /// multiple threads.
    for (const auto &[key, broker_info] : res["brokers"].items()) {
      BytesTransmittedToBrokers += broker_info.value("txbytes", (int64_t)0);
      TransmissionErrors += broker_info.value("txerrs", (int64_t)0);
      TxRequestRetries += broker_info.value("txretries", (int64_t)0);
    }

    /// assign the values to the ProducerStats struct in one step
    /// to ensure thread safety
    ProducerStats.BytesTransmittedToBrokers = BytesTransmittedToBrokers;
    ProducerStats.TransmissionErrors = TransmissionErrors;
    ProducerStats.TxRequestRetries = TxRequestRetries;

    break;

  case RdKafka::Event::EVENT_ERROR:
    applyKafkaErrorCode(event.err());
    break;

  default:
    break;
  }
}

// Implementation of DeliveryReportHandler override
void Producer::dr_cb(RdKafka::Message &message) {
  ++ProducerStats.TotalMsgDeliveryEvent;

  switch (message.status()) {
  case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
    ++ProducerStats.MsgStatusNotPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
    ++ProducerStats.MsgStatusPossiblyPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_PERSISTED:
    ++ProducerStats.MsgStatusPersisted;
    break;
  default:
    break;
  }

  if (message.err() != RdKafka::ErrorCode::ERR_NO_ERROR) {

    applyKafkaErrorCode(message.err());
    ++ProducerStats.MsgError;

  } else {
    ++ProducerStats.MsgDeliverySuccess;
  }
}

void Producer::applyKafkaErrorCode(RdKafka::ErrorCode ErrorCode) {

  // First log the error and its error string.
  LOG(KAFKA, Sev::Warning, "Rdkafka::Event::EVENT_ERROR [{}]: {}",
      static_cast<int>(ErrorCode), RdKafka::err2str(ErrorCode).c_str());
  XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR [%d]: %s\n", ErrorCode,
         RdKafka::err2str(ErrorCode).c_str());

  switch (ErrorCode) {
  case RdKafka::ErrorCode::ERR__TIMED_OUT:
    ++ProducerStats.ErrTimeout;
    break;
  case RdKafka::ErrorCode::ERR__TRANSPORT:
    ++ProducerStats.ErrTransport;
    break;
  case RdKafka::ErrorCode::ERR_BROKER_NOT_AVAILABLE:
    ++ProducerStats.ErrBrokerNotAvailable;
    break;
  case RdKafka::ErrorCode::ERR__UNKNOWN_TOPIC:
    ++ProducerStats.ErrTopic;
    break;
  case RdKafka::ErrorCode::ERR_TOPIC_EXCEPTION:
    ++ProducerStats.ErrTopic;
    break;
  case RdKafka::ErrorCode::ERR_TOPIC_AUTHORIZATION_FAILED:
    ++ProducerStats.ErrTopic;
    break;
  case RdKafka::ErrorCode::ERR__QUEUE_FULL:
    ++ProducerStats.ErrQueueFull;
    break;
  case RdKafka::ErrorCode::ERR__MSG_TIMED_OUT:
    ++ProducerStats.ErrMsgTimeout;
    break;
  case RdKafka::ErrorCode::ERR__AUTHENTICATION:
    ++ProducerStats.ErrAuth;
    break;
  case RdKafka::ErrorCode::ERR_MSG_SIZE_TOO_LARGE:
    ++ProducerStats.ErrMsgSizeTooLarge;
    break;
  case RdKafka::ErrorCode::ERR__UNKNOWN_PARTITION:
    ++ProducerStats.ErrUknownPartition;
    break;
  case RdKafka::ErrorCode::ERR_NO_ERROR:
    break;
  default:
    ++ProducerStats.ErrOther;
    break;
  }
}
