/** Copyright (C) 2016 European Spallation Source ERIC */

#ifndef NOKAFKA
#include <common/Producer.h>
#include <iostream>

Producer::Producer(std::string broker, bool enabled, std::string topicstr) {

  if (!enabled)
    return;

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
int Producer::produce() {

  std::string line = "producing...";
  RdKafka::ErrorCode resp = producer->produce(
      topic, -1, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
      const_cast<char *>(line.c_str()), line.size(), NULL, NULL);
  if (resp != RdKafka::ERR_NO_ERROR) {
    std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;
    return resp;
  }
  producer->poll(0);

  return 0;
}
#endif
