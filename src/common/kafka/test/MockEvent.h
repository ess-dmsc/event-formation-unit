#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <string>

class MockEvent : public RdKafka::Event {
public:
  MockEvent(const std::string &jsonStr,
            RdKafka::Event::Type eventType = RdKafka::Event::EVENT_STATS,
            RdKafka::ErrorCode errorCode = RdKafka::ERR_NO_ERROR)
      : jsonStr(jsonStr), eventType(eventType), errorCode(errorCode) {}

  Type type() const override { return eventType; }

  RdKafka::ErrorCode err() const override { return errorCode; }

  Severity severity() const override {
    return RdKafka::Event::EVENT_SEVERITY_INFO;
  }

  std::string fac() const override { return "MOCK"; }

  std::string str() const override { return jsonStr; }

  int throttle_time() const override { return 0; }

  std::string broker_name() const override { return "mock_broker"; }

  int broker_id() const override { return 0; }

  bool fatal() const override { return false; }

private:
  std::string jsonStr;
  RdKafka::Event::Type eventType;
  RdKafka::ErrorCode errorCode;
};
