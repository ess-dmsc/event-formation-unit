/** Copyright (C) 2016 European Spallation Source */

#include <librdkafka/rdkafkacpp.h>

class Producer {
public:
  Producer(std::string broker, bool enabled, std::string topicstr);
  int Produce(void);

private:
  std::string errstr;
  RdKafka::Conf *conf{0};
  RdKafka::Conf *tconf{0};
  RdKafka::Topic *topic{0};
  RdKafka::Producer *producer{0};
};
