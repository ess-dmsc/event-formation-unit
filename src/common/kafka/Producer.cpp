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
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
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
    ProducerStatCounters.config_errors++;
  }
  return configResult;
}

///
Producer::Producer(const std::string &Broker, const std::string &Topic,
                   std::vector<std::pair<std::string, std::string>> &Configs,
                   Statistics *Stats)
    : ProducerBase(), TopicName(Topic) {

  /// If Stats is nullptr, no internal stat counters will be be not registered
  if (Stats != nullptr) {
    // clang-format off
    Stats->create("kafka.config_errors", ProducerStatCounters.config_errors);
    Stats->create("kafka.produce_bytes_ok", ProducerStatCounters.produce_bytes_ok);
    Stats->create("kafka.produce_bytes_error", ProducerStatCounters.produce_bytes_error);
    Stats->create("kafka.produce_calls", ProducerStatCounters.produce_calls);
    Stats->create("kafka.produce_errors", ProducerStatCounters.produce_errors);
    
    /// librdkafka transmission stats
    Stats->create("kafka.brokers.tx_bytes", ProducerStatCounters.BytesTransmittedToBrokers);
    Stats->create("kafka.brokers.tx_req_retries", ProducerStatCounters.TxRequestRetries);
    
    /// librdkafka message stats
    Stats->create("kafka.msg.num_of_msg_in_queue", ProducerStatCounters.NumberOfMsgInQueue);
    Stats->create("kafka.msg.max_num_of_msg_in_queue", ProducerStatCounters.MaxNumOfMsgInQueue);
    Stats->create("kafka.msg.bytes_of_msg_in_queue", ProducerStatCounters.BytesOfMsgInQueue);
    Stats->create("kafka.msg.max_bytes_of_msg_in_queue", ProducerStatCounters.MaxBytesOfMsgInQueue);
    Stats->create("kafka.msg.delivery_success", ProducerStatCounters.MsgDeliverySuccess);
    Stats->create("kafka.msg.status_persisted", ProducerStatCounters.MsgStatusPersisted);
    Stats->create("kafka.msg.status_not_persisted", ProducerStatCounters.MsgStatusNotPersisted);
    Stats->create("kafka.msg.status_possibly_persisted", ProducerStatCounters.MsgStatusPossiblyPersisted);
    Stats->create("kafka.msg.msg_delivery_event", ProducerStatCounters.TotalMsgDeliveryEvent);
    
    /// librdkafka error stats
    Stats->create("kafka.error.msg_delivery", ProducerStatCounters.MsgError);
    Stats->create("kafka.error.transmission", ProducerStatCounters.TransmissionErrors);
    Stats->create("kafka.error.unknown_topic", ProducerStatCounters.ErrUnknownTopic);
    Stats->create("kafka.error.unknown_partition", ProducerStatCounters.ErrUknownPartition);
    Stats->create("kafka.error.queue_full", ProducerStatCounters.ErrQueueFull);
    Stats->create("kafka.error.timeout", ProducerStatCounters.ErrTimeout);
    Stats->create("kafka.error.msg_timeout", ProducerStatCounters.ErrMsgTimeout);
    Stats->create("kafka.error.transport", ProducerStatCounters.ErrTransport);
    Stats->create("kafka.error.authentication", ProducerStatCounters.ErrAuth);
    Stats->create("kafka.error.msg_size", ProducerStatCounters.ErrMsgSizeTooLarge);
    Stats->create("kafka.error.other", ProducerStatCounters.ErrOther);
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

  /// Register ourself as the event call back handler to receive events
  if (Config->set("event_cb", static_cast<RdKafka::EventCb *>(this),
                  ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set event_cb");
  }

  /// Register ourself as the delivery report handler to receive message
  /// delivery reports
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

  ProducerStatCounters.produce_calls++;

  if (error != RdKafka::ERR_NO_ERROR) {
    ProducerStatCounters.produce_errors++;
    ProducerStatCounters.produce_bytes_error += Buffer.size_bytes();
    applyKafkaErrorCode(error);

    LOG(KAFKA, Sev::Error, "Failed to produce message: {}",
        RdKafka::err2str(error));
    return error;
  }

  ProducerStatCounters.produce_bytes_ok += Buffer.size_bytes();

  return 0;
}

// Implementation of KafkaEventHandler override
void Producer::event_cb(RdKafka::Event &event) {
  nlohmann::json res;
  switch (event.type()) {

  case RdKafka::Event::EVENT_STATS:
    res = nlohmann::json::parse(event.str());
    ProducerStatCounters.NumberOfMsgInQueue = res["msg_cnt"].get<int64_t>();
    ProducerStatCounters.MaxNumOfMsgInQueue = res["msg_max"].get<int64_t>();
    ProducerStatCounters.BytesOfMsgInQueue = res["msg_size"].get<int64_t>();
    ProducerStatCounters.MaxBytesOfMsgInQueue +=
        res["msg_size_max"].get<int64_t>();

    /// reset broker related counters before looping over brokers
    /// and summing up the values
    ProducerStatCounters.BytesTransmittedToBrokers = 0;
    ProducerStatCounters.TransmissionErrors = 0;
    ProducerStatCounters.TxRequestRetries = 0;
    /// loop over brokers and sum up the values
    for (auto &broker : res["brokers"].items()) {
      const auto &broker_info = broker.value();
      ProducerStatCounters.BytesTransmittedToBrokers +=
          broker_info.value("txbytes", (int64_t)0);
      ProducerStatCounters.TransmissionErrors +=
          broker_info.value("txerrs", (int64_t)0);
      ProducerStatCounters.TxRequestRetries +=
          broker_info.value("txretries", (int64_t)0);
    }
    break;

  case RdKafka::Event::EVENT_ERROR:

    // First log the error and its error string.
    LOG(KAFKA, Sev::Warning, "Rdkafka::Event::EVENT_ERROR [{}]: {}",
        static_cast<int>(event.err()), RdKafka::err2str(event.err()).c_str());
    XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR [%d]: %s\n", event.err(),
           RdKafka::err2str(event.err()).c_str());

    applyKafkaErrorCode(event.err());

    break;

  default:
    break;
  }
}

// Implementation of DeliveryReportHandler override
void Producer::dr_cb(RdKafka::Message &message) {
  ++ProducerStatCounters.TotalMsgDeliveryEvent;

  switch (message.status()) {
  case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
    ++ProducerStatCounters.MsgStatusNotPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
    ++ProducerStatCounters.MsgStatusPossiblyPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_PERSISTED:
    ++ProducerStatCounters.MsgStatusPersisted;
    break;
  default:
    break;
  }

  if (message.err() != RdKafka::ErrorCode::ERR_NO_ERROR) {

    applyKafkaErrorCode(message.err());
    ++ProducerStatCounters.MsgError;

  } else {
    ++ProducerStatCounters.MsgDeliverySuccess;
  }
}