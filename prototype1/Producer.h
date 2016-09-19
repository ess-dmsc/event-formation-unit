#include <librdkafka/rdkafkacpp.h>

class Producer {
public:
  Producer(std::string topicstr);
  int Produce(int partition);

private:
  std::string tstr;
  std::string errstr;
  RdKafka::Conf *conf{0};
  RdKafka::Conf *tconf{0};
  RdKafka::Topic *topic{0};
  RdKafka::Producer *producer{0};
};
