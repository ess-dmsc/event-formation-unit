/** Copyright (C) 2016 - 2018 European Spallation Source ERIC */

#include <cassert>
#include <common/Producer.h>
#include <common/Log.h>
#include <common/Trace.h>
#include <libs/include/gccintel.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Producer::setConfig(std::string name, std::string value) {
  RdKafka::Conf::ConfResult configResult;
  configResult = conf->set(name, value, kafkaErrstr);
  LOG(Sev::Info, "Kafka set config {} to {}", name, value);
  if (configResult != RdKafka::Conf::CONF_OK) {
    LOG(Sev::Error, "Kafka Unable to set config {} to {}", name, value);
  }
  assert(configResult == RdKafka::Conf::CONF_OK); // compiles away in release build
}

///
// void Producer::dr_cb(RdKafka::Message &message) {
//   if (message.err() != RdKafka::ERR_NO_ERROR) {
//     stats.dr_errors++;
//     XTRACE(KAFKA, WAR, "RdKafkaDr error message (%d):  %s\n", message.err(), message.errstr().c_str());
//   } else {
//     stats.dr_noerrors++;
//     XTRACE(KAFKA, INF, "RdKafkaDr other message (%d):  %s\n", message.err(), message.errstr().c_str());
//   }
// }

///
void Producer::event_cb(RdKafka::Event &event) {
  switch (event.type()) {
     case RdKafka::Event::EVENT_ERROR:
       XTRACE(KAFKA, WAR, "Rdkafka::Event::EVENT_ERROR: %s\n", RdKafka::err2str(event.err()).c_str());
       stats.ev_errors++;
     break;
     default:
       XTRACE(KAFKA, INF, "RdKafka::Event:: %d: %s\n", event.type(), RdKafka::err2str(event.err()).c_str());
       stats.ev_others++;
     break;
  }
}

///
Producer::Producer(std::string broker, std::string topicstr) :
  ProducerBase(), topicString(topicstr) {

  conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  if (conf == nullptr) {
    LOG(Sev::Error, "Unable to create CONF_GLOBAL object");
    return;
  }

  if (tconf == nullptr) {
    LOG(Sev::Error, "Unable to create CONF_TOPIC object");
    return;
  }

  setConfig("metadata.broker.list", broker);
  setConfig("message.max.bytes", "10000000");
  setConfig("fetch.message.max.bytes", "10000000");
  setConfig("message.copy.max.bytes", "10000000");
  setConfig("queue.buffering.max.ms", "100");

  if (conf->set("event_cb", this, kafkaErrstr) != RdKafka::Conf::CONF_OK) {
    LOG(Sev::Error, "Kafka: unable to set event_cb");
  }

  // if (conf->set("dr_cb", this, kafkaErrstr) != RdKafka::Conf::CONF_OK) {
  //   LOG(Sev::Error, "Kafka: unable to set dr_cb");
  // }

  producer = RdKafka::Producer::create(conf, kafkaErrstr);
  if (!producer) {
    LOG(Sev::Error, "Failed to create producer: {}", kafkaErrstr);
    return;
  }

  topic = RdKafka::Topic::create(producer, topicstr, tconf, kafkaErrstr);
  if (!topic) {
    LOG(Sev::Error, "Failed to create topic: {}", kafkaErrstr);
    return;
  }
}

///
Producer::~Producer() {
  delete topic;
  delete producer;
  delete tconf;
  delete conf;
}

/** called to actually send data to Kafka cluster */
int Producer::produce(void *buffer, size_t bytes) {
  if (producer == nullptr || topic == nullptr) {
    return RdKafka::ERR_UNKNOWN;
  }
  RdKafka::ErrorCode resp = producer->produce(
      topic, -1, RdKafka::Producer::RK_MSG_COPY /* Copy payload */, buffer,
      bytes, NULL, NULL);

  producer->poll(0);
  if (resp != RdKafka::ERR_NO_ERROR) {
    XTRACE(KAFKA, DEB, "produce: %s", RdKafka::err2str(resp).c_str());
    stats.produce_fails++;
    return resp;
  }

  return 0;
}
