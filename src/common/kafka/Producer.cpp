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
    stats.config_errors++;
  }
  return configResult;
}

///
Producer::Producer(const std::string &Broker, const std::string &Topic,
                   std::vector<std::pair<std::string, std::string>> &Configs)
    : ProducerBase(), TopicName(Topic) {

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

  if (Config->set("event_cb", static_cast<RdKafka::EventCb *>(this),
                  ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set event_cb");
  }

  if (Config->set("dr_cb", static_cast<RdKafka::DeliveryReportCb *>(this),
                  ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set dr_cb");
  }

  // // Set message timeout to 5 seconds
  // Config->set("message.timeout.ms", "5000", ErrorMessage);

  // // Set retry backoff to 100ms
  // Config->set("retry.backoff.ms", "100", ErrorMessage);

  // Set the number of retries to 3
  // Config->set("retries", "3", ErrorMessage);

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
  KafkaProducer->produce(TopicName, -1, RdKafka::Producer::RK_MSG_COPY,
                         const_cast<std::uint8_t *>(Buffer.data()),
                         Buffer.size_bytes(), NULL, 0, MessageTimestampMS,
                         NULL);

  stats.produce_calls++;

  stats.produce_bytes_ok += Buffer.size_bytes();

  return 0;
}

// Implementation of KafkaEventHandler override
void Producer::event_cb(RdKafka::Event &event) {
  nlohmann::json res;
  switch (event.type()) {

  case RdKafka::Event::EVENT_STATS:
    res = nlohmann::json::parse(event.str());
    stats.NumberOfMsgInQueue = res["msg_cnt"].get<int64_t>();
    stats.MaxNumOfMsgInQueue = res["msg_max"].get<int64_t>();
    stats.BytesOfMsgInQueue = res["msg_size"].get<int64_t>();
    stats.MaxBytesOfMsgInQueue += res["msg_size_max"].get<int64_t>();
    for (auto &broker : res["brokers"].items()) {
      const auto &broker_info = broker.value();
      stats.BytesTransmittedToBrokers =
          broker_info.value("txbytes", (int64_t)0);
      stats.TransmissionErrors = broker_info.value("txerrs", (int64_t)0);
      stats.TxRequestRetries = broker_info.value("txretries", (int64_t)0);
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
  ++stats.TotalMsgDeliveryEvent;

  switch (message.status()) {
  case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
    ++stats.MsgStatusNotPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
    ++stats.MsgStatusPossiblyPersisted;
    break;
  case RdKafka::Message::MSG_STATUS_PERSISTED:
    ++stats.MsgStatusPersisted;
    break;
  default:
    break;
  }

  if (message.err() != RdKafka::ErrorCode::ERR_NO_ERROR) {

    applyKafkaErrorCode(message.err());
    ++stats.MsgError;

  } else {
    ++stats.MsgDeliverySuccess;
  }
}