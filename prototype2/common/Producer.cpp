/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Producer.h>
#include <iostream>
#include <libs/include/gccintel.h>

// using namespace BrightnESS::EventGenerator::FlatBufs::EFU;

Producer::Producer(std::string broker, std::string topicstr) {
  using namespace std;

  conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  if (conf == nullptr) {
    std::cerr << "Unable to created global Conf object" << std::endl;
    exit(1);
  }

  if (tconf == nullptr) {
    std::cerr << "Unable to created topic Conf object" << std::endl;
    exit(1);
  }

  conf->set("metadata.broker.list", broker, errstr);
  assert(errstr.empty() == true);
  conf->set("message.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);
  conf->set("fetch.message.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);
  conf->set("message.copy.max.bytes", "10000000", errstr);
  assert(errstr.empty() == true);

  producer = RdKafka::Producer::create(conf, errstr);
  if (!producer) {
    std::cerr << "Failed to create producer: " << errstr << std::endl;
    exit(1);
  }

  topic = RdKafka::Topic::create(producer, topicstr, tconf, errstr);
  if (!topic) {
    std::cerr << "Failed to create topic: " << errstr << std::endl;
    exit(1);
  }
}

/** called to actually send data to Kafka cluster */
int Producer::produce(char *buffer, int length) {
  RdKafka::ErrorCode resp = producer->produce(
      topic, -1, RdKafka::Producer::RK_MSG_COPY /* Copy payload */, buffer,
      length, NULL, NULL);
  if (resp != RdKafka::ERR_NO_ERROR) {
    std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;
    return resp;
  }
  producer->poll(0);

  return 0;
}
