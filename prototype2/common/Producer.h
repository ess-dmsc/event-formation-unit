/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for sending data to Kafka broker
///
//===----------------------------------------------------------------------===//

#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <common/Buffer.h>
#include <functional>

class ProducerBase {
public:
  ProducerBase() = default;
  virtual ~ProducerBase() = default;

  // \todo deprecate this function in favor of encapsulated buffer
  virtual int produce(char *buffer, size_t length) = 0;

  inline void produce2(const Buffer<uint8_t>& buffer)
  {
    this->produce(reinterpret_cast<char*>(buffer.buffer), buffer.size);
  }
};

class Producer : public ProducerBase {
public:
  /// \brief Construct a producer object.
  /// \param broker 'URL' specifying host and port, example "127.0.0.1:9009"
  /// \param topicstr Name of Kafka topic according to agreement, example
  /// "T-REX_detectors"
  Producer(std::string broker, std::string topicstr);

  ~Producer();

  /** \brief Function called to send data to a broker
   *  @param buffer Pointer to char buffer containing data to be tx'ed
   *  @param length Size of buffer data in bytes
   */
  int produce(char *buffer, size_t length) override;

private:
  std::string errstr;
  RdKafka::Conf *conf{0};
  RdKafka::Conf *tconf{0};
  RdKafka::Topic *topic{0};
  RdKafka::Producer *producer{0};
};

using ProducerCallback = std::function<void(Buffer<uint8_t>)>;
