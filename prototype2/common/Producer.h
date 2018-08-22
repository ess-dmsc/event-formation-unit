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

class ProducerBase {
public:
  ProducerBase() = default;
  virtual ~ProducerBase() = default;
  virtual int produce(char *buffer, int length) = 0;
};

class Producer : public ProducerBase {
public:
  /// \brief Construct a producer object.
  /// \param broker 'URL' specifying host and port, example "127.0.0.1:9009"
  /// \param topicstr Name of Kafka topic according to agreement, example
  /// "T-REX_detectors"
  Producer(std::string broker, std::string topicstr);

  ~Producer();

  /// \brief Function called to send data to a broker
  /// \param buffer Pointer to char buffer containing data to be tx'ed
  /// \param length Size of buffer data in bytes
  int produce(char *buffer, int length) override;

  /// \brief Kafka callback function for delivery reports
  class DeliveryCallback : public RdKafka::DeliveryReportCb {
  public:
    void dr_cb(RdKafka::Message &message);

    struct {
      uint64_t dr_error;
      uint64_t dr_noerror;
    } stats = {};
  };

  /// \brief Kafka callback function for events
  class EventCallback : public RdKafka::EventCb {
  public:
    void event_cb(RdKafka::Event &event);

    struct {
      uint64_t ev_error;
      uint64_t ev_other;
      // uint64_t ev_log;
      // uint64_t ev_stats;
      // uint64_t ev_throttle;
    } stats = {};
  };

  DeliveryCallback delivery_callback;
  EventCallback event_callback;

private:
  std::string kafkaErrstr;
  std::string topicString;
  RdKafka::Conf *conf{0};
  RdKafka::Conf *tconf{0};
  RdKafka::Topic *topic{0};
  RdKafka::Producer *producer{0};
};
