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

// void Producer::dr_cb(RdKafka::Message &message) {
//   if (message.status() == RdKafka::Message::MSG_STATUS_NOT_PERSISTED) {
//     std::cerr << "Message not persisted: " << message.errstr() << std::endl;
//   } else if (message.status() ==
//              RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED) {
//     std::cerr << "Message possibly persisted: " << message.errstr()
//               << std::endl;
//   } else if (message.status() == RdKafka::Message::MSG_STATUS_PERSISTED) {
//     std::cerr << "Message persisted: " << message.errstr() << std::endl;
//   }

//   if (message.err()) {
//     std::cerr << "Message delivery failed: " << message.errstr() << std::endl;
//   } else {
//     std::cout << "Message delivered to topic " << message.topic_name() << " ["
//               << message.partition() << "] at offset " << message.offset()
//               << std::endl;
//   }
// }

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

  // if (Config->set("event_cb", this, ErrorMessage) != RdKafka::Conf::CONF_OK)
  // {
  //   LOG(KAFKA, Sev::Error, "Kafka: unable to set event_cb");
  // }

  if (Config->set("dr_cb", this, ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set dr_cb");
  }

  // Set message timeout to 5 seconds
  Config->set("message.timeout.ms", "5000", ErrorMessage);

  // Set retry backoff to 100ms
  Config->set("retry.backoff.ms", "100", ErrorMessage);

  // Set the number of retries to 3
  Config->set("retries", "3", ErrorMessage);

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
  RdKafka::ErrorCode resp = KafkaProducer->produce(
      TopicName, -1, RdKafka::Producer::RK_MSG_COPY,
      const_cast<std::uint8_t *>(Buffer.data()), Buffer.size_bytes(), NULL, 0,
      MessageTimestampMS, NULL);

  stats.produce_calls++;

  // poll for events in the event queue and triggers callbacks on them
  KafkaProducer->poll(0);

  /// \todo: this is probably not happen because we have async producer with
  /// kafka
  if (resp != RdKafka::ERR_NO_ERROR) {
    if (resp == RdKafka::ERR__UNKNOWN_TOPIC) {
      stats.err_unknown_topic++;
    } else if (resp == RdKafka::ERR__QUEUE_FULL) {
      stats.err_queue_full++;
    } else {
      stats.err_other++;
    }

    XTRACE(KAFKA, DEB, "produce: %s", RdKafka::err2str(resp).c_str());
    stats.produce_bytes_error += Buffer.size_bytes();
    stats.produce_errors++;
    return resp;
  } else {
    stats.produce_bytes_ok += Buffer.size_bytes();
    // stats.produce_no_errors++;
  }

  return 0;
}

// Implementation of KafkaEventHandler override
void KafkaEventHandler::event_cb(RdKafka::Event &event) {
  nlohmann::json res;
  switch (event.type()) {
  case RdKafka::Event::EVENT_STATS:
    res = nlohmann::json::parse(event.str());
    NumberOfMessageInQueue += res["msg_cnt"].get<int64_t>();
    SizeOfMessageInQueue += res["msg_size"].get<int64_t>();
    for (auto &broker : res["brokers"].items()) {
      const auto &broker_info = broker.value();
      BytesTransmittedToBrokers += broker_info.value("txbytes", (int64_t)0);
      TransmissionErrors += broker_info.value("txerrs", (int64_t)0);
      TxRequestRetries += broker_info.value("txerrs", (int64_t)0);
    }
    break;
  case RdKafka::Event::EVENT_ERROR:

    // First log the error and it's erro string.
    LOG(KAFKA, Sev::Warning, "Rdkafka::Event::EVENT_ERROR [{}]: {}",
        event.err(), RdKafka::err2str(event.err()).c_str());
    XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR [%d]: %s\n", event.err(),
           RdKafka::err2str(event.err()).c_str());

    switch (event.err()) {
    case RdKafka::ErrorCode::ERR__TIMED_OUT:
      ErrTimeout++;
      break;
    case RdKafka::ErrorCode::ERR__TRANSPORT:
      ErrTransport++;
      break;
    case RdKafka::ErrorCode::ERR_BROKER_NOT_AVAILABLE:
      ErrBrokerNotAvailable++;
      break;
    case RdKafka::ErrorCode::ERR__UNKNOWN_TOPIC:
      ErrUnknownTopic++;
      break;
    case RdKafka::ErrorCode::ERR__QUEUE_FULL:
      ErrQueueFull++;
      break;
    case RdKafka::ErrorCode::ERR_NO_ERROR:
      break;
    default:
      ErrOther++;
      break;
    }
    break;
  default:
    break;
  }
}

// Implementation of DeliveryReportHandler override
void DeliveryReportHandler::dr_cb(RdKafka::Message &message) {
  ++totalCount;
  if (message.err() != RdKafka::ERR_NO_ERROR) {
    ++errorCount;
  } else {
    ++successCount;
  }
  // ...existing code...
}