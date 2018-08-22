/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Producer.h>
#include <common/Log.h>
#include <common/Trace.h>
#include <libs/include/gccintel.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB


void Producer::DeliveryCallback::dr_cb(RdKafka::Message &message) {
  XTRACE(KAFKA, INF, "**** RdKafka Delivery Callback ****\n");
  if (message.err() != RdKafka::ERR_NO_ERROR) {
    stats.dr_error++;
    XTRACE(KAFKA, WAR, "RdKafkaDr error message (%d):  %s\n", message.err(), message.errstr().c_str());
  } else {
    stats.dr_noerror++;
    XTRACE(KAFKA, INF, "RdKafkaDr other message (%d):  %s\n", message.err(), message.errstr().c_str());
  }
}


void Producer::EventCallback::event_cb(RdKafka::Event &event) {
  switch (event.type()) {
     case RdKafka::Event::EVENT_ERROR:
       printf("**** Rdkafka::Event::EVENT_ERROR: %s\n", RdKafka::err2str(event.err()).c_str());
       stats.ev_error++;
     break;
     default:
       printf("**** RdKafka::EventId: %d: %s\n", event.type(), RdKafka::err2str(event.err()).c_str());
       stats.ev_other++;
     break;
  }
}


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

  RdKafka::Conf::ConfResult __attribute__((unused)) configResult;
  configResult = conf->set("metadata.broker.list", broker, kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);
  configResult = conf->set("message.max.bytes", "10000000", kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);
  configResult = conf->set("fetch.message.max.bytes", "10000000", kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);
  configResult = conf->set("message.copy.max.bytes", "10000000", kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);
  configResult = conf->set("queue.buffering.max.ms", "100", kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);


  configResult = conf->set("event_cb", &event_callback, kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);


  configResult = conf->set("dr_cb", &delivery_callback, kafkaErrstr);
  assert(configResult == RdKafka::Conf::CONF_OK);

  producer = RdKafka::Producer::create(conf, kafkaErrstr);
  if (!producer) {
    LOG(Sev::Error, "Failed to create producer: {}", kafkaErrstr);
    /** \todo add logging to Greylog */
    return;
  }

  topic = RdKafka::Topic::create(producer, topicstr, tconf, kafkaErrstr);
  if (!topic) {
    LOG(Sev::Error, "Failed to create topic: {}", kafkaErrstr);
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

  XTRACE(DATA, DEB, "produce returns %d\n", resp);
  producer->poll(0);
  if (resp != RdKafka::ERR_NO_ERROR) {

    XTRACE(DATA, DEB, "produce: %s\n", RdKafka::err2str(resp).c_str());
    LOG(Sev::Error, "Produce failed: {}", RdKafka::err2str(resp));
    return resp;
  }

  return 0;
}
