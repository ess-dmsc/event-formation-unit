/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for sending data to Kafka broker
 */

#pragma once
#ifndef NOKAFKA
#include <librdkafka/rdkafkacpp.h>

class Producer {
public:
  /** @brief Construct a producer object.
   * @param broker 'URL' specifying host and port, example "127.0.0.1:9009"
   * @param enabled If false do not send data to broker when calling produce()
   * @param topicstr Name of Kafka topic according to agreement, example
   * "T-REX_detectors"
  */
  Producer(std::string broker, bool enabled, std::string topicstr);

  /** @brief Function called to send data to a broker
   *  @todo currently transmits a fixed hello world string, should take a
   *  data object reference or similar.
   */
  int produce();

  /** @brief Function called to send data to a broker
   *  @param buffer Pointer to char buffer containing data to be tx'ed
   *  @param length Size of buffer data in bytes
   */
  int produce(char * buffer, int length);

private:
  std::string errstr;
  RdKafka::Conf *conf{0};
  RdKafka::Conf *tconf{0};
  RdKafka::Topic *topic{0};
  RdKafka::Producer *producer{0};
};
#endif
