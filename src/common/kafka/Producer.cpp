// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Kafka producer - wrapper for librdkafka
///
/// See https://github.com/edenhill/librdkafka
//===----------------------------------------------------------------------===//

#include <cassert>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/kafka/Producer.h>
#include <common/system/gccintel.h>
#include <nlohmann/json.hpp>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

RdKafka::Conf::ConfResult Producer::setConfig(std::string Key,
                                              std::string Value) {
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
void Producer::event_cb(RdKafka::Event &event) {
  nlohmann::json res;
  switch (event.type()) {
  case RdKafka::Event::EVENT_STATS:
    res = nlohmann::json::parse(event.str());
    stats.librdkafka_msg_cnt = res["msg_cnt"].get<int64_t>();
    stats.librdkafka_msg_size = res["msg_size"].get<int64_t>();
    break;
  case RdKafka::Event::EVENT_ERROR:
    LOG(KAFKA, Sev::Warning, "Rdkafka::Event::EVENT_ERROR: {}",
        RdKafka::err2str(event.err()).c_str());
    XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR: %s\n",
           RdKafka::err2str(event.err()).c_str());
    stats.ev_errors++;
    break;
  default:
    XTRACE(KAFKA, INF, "RdKafka::Event:: %d: %s\n", event.type(),
           RdKafka::err2str(event.err()).c_str());
    stats.ev_others++;
    break;
  }
}

///
Producer::Producer(std::string Broker, std::string Topic,
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

  if (Config->set("event_cb", this, ErrorMessage) != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka: unable to set event_cb");
  }

  // if (conf->set("dr_cb", this, ErrorMessage) != RdKafka::Conf::CONF_OK) {
  //   LOG(KAFKA, Sev::Error, "Kafka: unable to set dr_cb");
  // }

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

int Producer::produce(nonstd::span<const std::uint8_t> Buffer,
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
    stats.produce_no_errors++;
  }

  return 0;
}
