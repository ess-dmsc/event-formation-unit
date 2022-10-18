// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Kafka producer - werapper for librdkafka
///
/// See https://github.com/edenhill/librdkafka
//===----------------------------------------------------------------------===//

#include <cassert>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/kafka/Producer.h>
#include <common/system/gccintel.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Producer::setConfig(std::string Key, std::string Value) {
  RdKafka::Conf::ConfResult configResult;
  configResult = Config->set(Key, Value, ErrorMessage);
  LOG(KAFKA, Sev::Info, "Kafka set config {} to {}", Key, Value);
  if (configResult != RdKafka::Conf::CONF_OK) {
    LOG(KAFKA, Sev::Error, "Kafka Unable to set config {} to {}", Key, Value);
  }
  assert(configResult ==
         RdKafka::Conf::CONF_OK); // compiles away in release build
}

///
void Producer::event_cb(RdKafka::Event &event) {
  switch (event.type()) {
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
                   std::vector<std::pair<std::string, std::string>> & Configs)
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

  for (auto & Config : Configs) {
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

/** called to actually send data to Kafka cluster */
int Producer::produce(nonstd::span<const std::uint8_t> Buffer,
                      std::int64_t MessageTimestampMS) {
  if (KafkaProducer == nullptr || KafkaTopic == nullptr) {
    return RdKafka::ERR_UNKNOWN;
  }

  RdKafka::ErrorCode resp = KafkaProducer->produce(
      TopicName, -1, RdKafka::Producer::RK_MSG_COPY,
      const_cast<std::uint8_t *>(Buffer.data()), Buffer.size_bytes(), NULL, 0,
      MessageTimestampMS, NULL);

  stats.produce_calls++;

  KafkaProducer->poll(0);
  if (resp != RdKafka::ERR_NO_ERROR) {
    XTRACE(KAFKA, DEB, "produce: %s", RdKafka::err2str(resp).c_str());
    stats.produce_fails++;
    return resp;
  }

  return 0;
}
