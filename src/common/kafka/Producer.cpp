// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Kafka producer - wrapper for librdkafka
///
/// \note: See https://github.com/edenhill/librdkafka
///
/// For more information the available producer statistics see:
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

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

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
    StatCounters.ErrConfig++;
  }
  return configResult;
}

///
Producer::Producer(const std::string &Broker, const std::string &Topic,
                   std::vector<std::pair<std::string, std::string>> &Configs,
                   Statistics &Stats, const std::string &Name)
    : ProducerBase(), TopicName(Topic),
      StatCounters(Stats, "producer." + Name) {

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

int Producer::produce(const nonstd::span<const uint8_t> &Buffer,
                      int64_t MessageTimestampMS) {

  if (KafkaProducer == nullptr || KafkaTopic == nullptr) {
    return RdKafka::ERR_UNKNOWN;
  }

  // non-blocking, copies the buffer to kafka thread for transfer
  auto error = KafkaProducer->produce(
      TopicName, -1, RdKafka::Producer::RK_MSG_COPY,
      const_cast<uint8_t *>(Buffer.data()), Buffer.size_bytes(), NULL, 0,
      MessageTimestampMS, NULL);

  KafkaProducer->poll(0);

  StatCounters.produce_calls++;

  if (error != RdKafka::ERR_NO_ERROR) {
    StatCounters.produce_errors++;
    StatCounters.produce_bytes_error += Buffer.size_bytes();
    applyKafkaErrorCode(error);

    LOG(KAFKA, Sev::Error, "Failed to produce message: {}",
        RdKafka::err2str(error));
    return error;
  }

  StatCounters.produce_bytes_ok += Buffer.size_bytes();

  return 0;
}

// Implementation of KafkaEventHandler override
void Producer::event_cb(RdKafka::Event &event) {
  nlohmann::json res;

  /// initialize variable to sum up the values later when looping over brokers
  int64_t TransmissionErrors = 0;
  int64_t BytesTransmittedToBrokers = 0;
  int64_t TxRequestRetries = 0;

  switch (event.type()) {
  case RdKafka::Event::EVENT_STATS:
    StatCounters.StatsEventCounter++;

    res = nlohmann::json::parse(event.str());
    StatCounters.NumberOfMsgInQueue = res["msg_cnt"].get<int64_t>();
    StatCounters.MaxNumOfMsgInQueue = res["msg_max"].get<int64_t>();
    StatCounters.BytesOfMsgInQueue = res["msg_size"].get<int64_t>();
    StatCounters.MaxBytesOfMsgInQueue += res["msg_size_max"].get<int64_t>();

    /// \note loop over brokers and sum up the values into local variables
    /// before assigning them to the ProducerStats struct.
    /// This is to improve thread safety because the ProducerStats struct
    /// is registered into the Statistics object and can be accessed from
    /// multiple threads.
    for (const auto &[key, broker_info] : res["brokers"].items()) {
      BytesTransmittedToBrokers += broker_info.value("txbytes", (int64_t)0);
      TransmissionErrors += broker_info.value("txerrs", (int64_t)0);
      TxRequestRetries += broker_info.value("txretries", (int64_t)0);
    }

    /// assign the values to the ProducerStats struct in one step
    /// to ensure thread safety
    StatCounters.BytesTransmittedToBrokers = BytesTransmittedToBrokers;
    StatCounters.TransmissionErrors = TransmissionErrors;
    StatCounters.TxRequestRetries = TxRequestRetries;

    break;

  case RdKafka::Event::EVENT_ERROR:
    StatCounters.ErrorEventCounter++;
    applyKafkaErrorCode(event.err());
    break;

  default:
    break;
  }
}

// Implementation of DeliveryReportHandler override
void Producer::dr_cb(RdKafka::Message &message) {
  StatCounters.TotalMsgDeliveryEvent++;

  switch (message.status()) {
  case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
    StatCounters.MsgStatusNotPersisted++;
    break;
  case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
    StatCounters.MsgStatusPossiblyPersisted++;
    break;
  case RdKafka::Message::MSG_STATUS_PERSISTED:
    StatCounters.MsgStatusPersisted++;
    break;
  default:
    break;
  }

  if (message.err() != RdKafka::ErrorCode::ERR_NO_ERROR) {
    auto error = message.err();
    applyKafkaErrorCode(error);
    StatCounters.MsgError++;

  } else {
    StatCounters.MsgDeliverySuccess++;
  }
}

void Producer::applyKafkaErrorCode(RdKafka::ErrorCode ErrorCode) {

  // First log the error and its error string.
  LOG(KAFKA, Sev::Warning, "Rdkafka error occurred: [{}] {}",
      static_cast<int>(ErrorCode), RdKafka::err2str(ErrorCode).c_str());
  XTRACE(KAFKA, WAR, "Rdkafka error occurred: [%d] %s\n", ErrorCode,
         RdKafka::err2str(ErrorCode).c_str());

  switch (ErrorCode) {
  case RdKafka::ErrorCode::ERR__TIMED_OUT:
    StatCounters.ErrTimeout++;
    break;
  case RdKafka::ErrorCode::ERR__TRANSPORT:
    StatCounters.ErrTransport++;
    break;
  case RdKafka::ErrorCode::ERR_BROKER_NOT_AVAILABLE:
    StatCounters.ErrBrokerNotAvailable++;
    break;
  case RdKafka::ErrorCode::ERR__UNKNOWN_TOPIC:
    StatCounters.ErrTopic++;
    break;
  case RdKafka::ErrorCode::ERR_TOPIC_EXCEPTION:
    StatCounters.ErrTopic++;
    break;
  case RdKafka::ErrorCode::ERR_TOPIC_AUTHORIZATION_FAILED:
    StatCounters.ErrTopic++;
    break;
  case RdKafka::ErrorCode::ERR__QUEUE_FULL:
    StatCounters.ErrQueueFull++;
    break;
  case RdKafka::ErrorCode::ERR__MSG_TIMED_OUT:
    StatCounters.ErrMsgTimeout++;
    break;
  case RdKafka::ErrorCode::ERR__AUTHENTICATION:
    StatCounters.ErrAuth++;
    break;
  case RdKafka::ErrorCode::ERR_MSG_SIZE_TOO_LARGE:
    StatCounters.ErrMsgSizeTooLarge++;
    break;
  case RdKafka::ErrorCode::ERR__UNKNOWN_PARTITION:
    StatCounters.ErrUknownPartition++;
    break;
  case RdKafka::ErrorCode::ERR_NO_ERROR:
    break;
  default:
    StatCounters.ErrOther++;
    break;
  }
}