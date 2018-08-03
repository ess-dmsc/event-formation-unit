/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Producer.h>
#include <common/Log.h>
#include <libs/include/gccintel.h>

Producer::Producer(std::string broker, std::string topicstr) : ProducerBase() {

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

  conf->set("metadata.broker.list", broker, errstr);
  assert(errstr.empty() == true);
  conf->set("message.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);
  conf->set("fetch.message.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);
  conf->set("message.copy.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);
  conf->set("queue.buffering.max.ms", "100", errstr);
  assert(errstr.empty() == true);

  producer = RdKafka::Producer::create(conf, errstr);
  if (!producer) {
    LOG(Sev::Error, "Failed to create producer: {}", errstr);
    /** \todo add logging to Greylog */
    return;
  }

  topic = RdKafka::Topic::create(producer, topicstr, tconf, errstr);
  if (!topic) {
    LOG(Sev::Error, "Failed to create topic: {}", errstr);
    /** \todo add logging to Greylog */
    return;
  }
}

Producer::~Producer() {
  delete topic;
  delete producer;
  delete tconf;
  delete conf;
}

/** called to actually send data to Kafka cluster */
int Producer::produce(char *buffer, int length) {
  if (producer == nullptr || topic == nullptr) {
    return RdKafka::ERR_UNKNOWN;
  }
  RdKafka::ErrorCode resp = producer->produce(
      topic, -1, RdKafka::Producer::RK_MSG_COPY /* Copy payload */, buffer,
      length, NULL, NULL);
  if (resp != RdKafka::ERR_NO_ERROR) {
    LOG(Sev::Error, "Produce failed: {}", RdKafka::err2str(resp));
    return resp;
  }
  producer->poll(0);

  return 0;
}
