// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Unit test for Kafka Producer
//===----------------------------------------------------------------------===//

#include "KafkaMocks.h"
#include <common/Statistics.h>
#include <common/kafka/KafkaConfig.h>
#include <common/kafka/Producer.h>
#include <common/testutils/TestBase.h>
#include <cstring>
#include <dlfcn.h>
#include <librdkafka/rdkafkacpp.h>
#include <trompeloeil.hpp>

KafkaConfig KafkaCfg{""};

using trompeloeil::_;

namespace trompeloeil {
template <>
void reporter<specialized>::send(severity s, char const *file,
                                 unsigned long line, const char *msg) {
  if (s == severity::fatal) {
    std::ostringstream os;
    if (line != 0U) {
      os << file << ':' << line << '\n';
    }
    throw expectation_violation(os.str() + msg);
  }
  ADD_FAILURE_AT(file, line) << msg;
}
} // namespace trompeloeil

class ProducerStandIn : public Producer {
public:
  ProducerStandIn(const std::string &Broker, const std::string &Topic,
                  Statistics *Stats = nullptr)
      : Producer(Broker, Topic, KafkaCfg.CfgParms, Stats) {}
  using Producer::Config;
  using Producer::KafkaProducer;
  using Producer::KafkaTopic;
  using Producer::TopicConfig;
};

class ProducerTest : public TestBase {
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(ProducerTest, ConstructorOK) {

  ProducerStandIn prod{"nobroker", "notopic"};
  std::vector<unsigned char> DataBuffer(10);
  int ret = prod.produce(DataBuffer, time(nullptr) * 1000);
  ASSERT_NE(prod.Config, nullptr);
  ASSERT_NE(prod.TopicConfig, nullptr);
  ASSERT_NE(prod.KafkaTopic, nullptr);
  ASSERT_NE(prod.KafkaProducer, nullptr);
  ASSERT_EQ(ret, RdKafka::ERR_NO_ERROR);
  EXPECT_EQ(prod.getStats().produce_bytes_error, 0);
  EXPECT_EQ(prod.getStats().produce_bytes_ok, 10);
  EXPECT_EQ(prod.getStats().produce_calls, 1);
  EXPECT_EQ(prod.getStats().produce_errors, 0);
  EXPECT_EQ(prod.getStats().ErrTimeout, 0);
  EXPECT_EQ(prod.getStats().ErrTransport, 0);
  EXPECT_EQ(prod.getStats().ErrBrokerNotAvailable, 0);
  EXPECT_EQ(prod.getStats().ErrTopic, 0);
  EXPECT_EQ(prod.getStats().ErrQueueFull, 0);
  EXPECT_EQ(prod.getStats().ErrOther, 0);
  EXPECT_EQ(prod.getStats().ErrMsgTimeout, 0);
  EXPECT_EQ(prod.getStats().ErrAuth, 0);
  EXPECT_EQ(prod.getStats().ErrMsgSizeTooLarge, 0);
  EXPECT_EQ(prod.getStats().ErrUknownPartition, 0);
  EXPECT_EQ(prod.getStats().TransmissionErrors, 0);
  EXPECT_EQ(prod.getStats().MsgError, 0);
  EXPECT_EQ(prod.getStats().MsgDeliverySuccess, 0);
  EXPECT_EQ(prod.getStats().MsgStatusPersisted, 0);
  EXPECT_EQ(prod.getStats().MsgStatusPossiblyPersisted, 0);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 0);
  EXPECT_EQ(prod.getStats().TotalMsgDeliveryEvent, 0);
  EXPECT_EQ(prod.getStats().NumberOfMsgInQueue, 0);
  EXPECT_EQ(prod.getStats().MaxNumOfMsgInQueue, 0);
  EXPECT_EQ(prod.getStats().BytesOfMsgInQueue, 0);
  EXPECT_EQ(prod.getStats().MaxBytesOfMsgInQueue, 0);
  EXPECT_EQ(prod.getStats().BytesTransmittedToBrokers, 0);
  EXPECT_EQ(prod.getStats().TxRequestRetries, 0);
}

TEST_F(ProducerTest, ConstructorCreatesStatsEntries) {
  Statistics Stats;

  ProducerStandIn prod{"nobroker", "notopic", &Stats};

  EXPECT_EQ(Stats.valueByName("kafka.config_errors"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.produce_bytes_ok"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.produce_bytes_error"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.produce_calls"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.produce_errors"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.brokers.tx_bytes"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.brokers.tx_req_retries"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.num_of_msg_in_queue"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.max_num_of_msg_in_queue"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.bytes_of_msg_in_queue"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.max_bytes_of_msg_in_queue"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.delivery_success"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.status_persisted"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.status_not_persisted"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.status_possibly_persisted"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.msg.msg_delivery_event"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.msg_delivery"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.transmission"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.unknown_topic"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.unknown_partition"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.queue_full"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.timeout"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.msg_timeout"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.transport"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.authentication"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.msg_size"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error.other"), 0);
}

TEST_F(ProducerTest, ConfigError) {
  ProducerStandIn prod{"nobroker", "notopic"};
  auto Res = prod.setConfig("queue.buffering.max.ms", "101");
  ASSERT_EQ(Res, RdKafka::Conf::CONF_OK);
  ASSERT_EQ(prod.getStats().config_errors, 0);

  Res = prod.setConfig("this.does.not.exist", "so.this.has.no.meaning");
  ASSERT_NE(Res, RdKafka::Conf::CONF_OK);
  ASSERT_EQ(prod.getStats().config_errors, 1);
}

TEST_F(ProducerTest, CreateConfFail1) {
  ProducerStandIn prod{"nobroker", "notopic"};
  prod.KafkaProducer.reset(nullptr);
  std::array<uint8_t, 10> Data;
  int ret = prod.produce(Data, 10);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);
}

TEST_F(ProducerTest, CreateConfFail2) {
  ProducerStandIn prod{"nobroker", "notopic"};
  prod.KafkaTopic.reset(nullptr);
  std::array<uint8_t, 10> Data;
  int ret = prod.produce(Data, 10);
  ASSERT_EQ(ret, RdKafka::ERR_UNKNOWN);
}

TEST_F(ProducerTest, ProducerFail) {
  ProducerStandIn prod{"nobroker", "notopic"};
  auto *TempProducer = new MockProducer;
  RdKafka::ErrorCode ReturnValue = RdKafka::ERR__STATE;
  REQUIRE_CALL(*TempProducer, produce(_, _, _, _, _, _, _, _, _))
      .TIMES(1)
      .RETURN(ReturnValue);
  REQUIRE_CALL(*TempProducer, poll(_)).TIMES(1).RETURN(0);
  prod.KafkaProducer.reset(TempProducer);
  std::uint8_t SomeData[20];
  ASSERT_EQ(prod.getStats().produce_errors, 0);
  int ret = prod.produce(SomeData, 999);
  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(prod.getStats().produce_errors, 1);
}

TEST_F(ProducerTest, ProducerSuccess) {
  ProducerStandIn prod{"nobroker", "notopic"};
  auto *TempProducer = new MockProducer;
  RdKafka::ErrorCode ReturnValue = RdKafka::ERR_NO_ERROR;
  REQUIRE_CALL(*TempProducer, produce(_, _, _, _, _, _, _, _, _))
      .TIMES(1)
      .RETURN(ReturnValue);
  REQUIRE_CALL(*TempProducer, poll(_)).TIMES(1).RETURN(0);
  prod.KafkaProducer.reset(TempProducer);
  unsigned int NrOfBytes{200};
  auto SomeData = std::make_unique<unsigned char[]>(NrOfBytes);
  ASSERT_EQ(prod.getStats().produce_errors, 0);
  ASSERT_EQ(prod.getStats().produce_bytes_ok, 0);

  int ret = prod.produce({SomeData.get(), NrOfBytes}, 999);
  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(prod.getStats().produce_errors, 0);
  ASSERT_EQ(prod.getStats().produce_bytes_ok, 200);
}

TEST_F(ProducerTest, ProducerFailDueToSize) {
  KafkaConfig KafkaCfg2("");
  ASSERT_EQ(KafkaCfg2.CfgParms.size(), 5);
  Producer prod{"nobroker", "notopic", KafkaCfg2.CfgParms};
  auto DataPtr = std::make_unique<unsigned char>();
  int ret = prod.produce({DataPtr.get(), 100000000}, 1);
  ASSERT_EQ(ret, RdKafka::ERR_MSG_SIZE_TOO_LARGE);
}

TEST_F(ProducerTest, EventCbIncreasesCounters) {
  ProducerStandIn prod{"nobroker", "notopic"};

  std::string fakeEventJson = R"({
    "msg_cnt": 10,
    "msg_max": 20,
    "msg_size": 1000,
    "msg_size_max": 2000,
    "brokers": {
      "broker1": {
        "txbytes": 500,
        "txerrs": 2,
        "txretries": 1
      },
      "broker2": {
        "txbytes": 300,
        "txerrs": 1,
        "txretries": 2
      }
    }
  })";

  MockEvent fakeEvent(fakeEventJson);
  prod.event_cb(fakeEvent);

  EXPECT_EQ(prod.getStats().NumberOfMsgInQueue, 10);
  EXPECT_EQ(prod.getStats().MaxNumOfMsgInQueue, 20);
  EXPECT_EQ(prod.getStats().BytesOfMsgInQueue, 1000);
  EXPECT_EQ(prod.getStats().MaxBytesOfMsgInQueue, 2000);
  EXPECT_EQ(prod.getStats().BytesTransmittedToBrokers, 800);
  EXPECT_EQ(prod.getStats().TransmissionErrors, 3);
  EXPECT_EQ(prod.getStats().TxRequestRetries, 3);
}

TEST_F(ProducerTest, EventCbProcessesErrorsAndLogs) {
  ProducerStandIn prod{"nobroker", "notopic"};
  std::string errorEventJson = R"({"error": "mock error"})";

  /// Create a logger factory which initializes the logger mock
  /// if this object goes out of scope, the logger mock is destroyed
  auto LoggerFactory = MockLoggerFactory();

  /// Register the Mocked logger to be chekced
  /// for called 8 times with the correct log
  REQUIRE_CALL(LoggerFactory.getMockedLogger(), log(_, _))
      .TIMES(8)
      .WITH(_1 == "KAFKA" &&
            _2.find("Rdkafka::Event::EVENT_ERROR") != std::string::npos);

  MockEvent errorEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                       RdKafka::ERR__TIMED_OUT);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrTimeout, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR_BROKER_NOT_AVAILABLE);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrBrokerNotAvailable, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR__TRANSPORT);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrTransport, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR__AUTHENTICATION);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrAuth, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR__MSG_TIMED_OUT);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrMsgTimeout, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR__UNKNOWN_TOPIC);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrTopic, 1);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR_TOPIC_AUTHORIZATION_FAILED);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrTopic, 2);

  errorEvent = MockEvent(errorEventJson, RdKafka::Event::EVENT_ERROR,
                         RdKafka::ERR_TOPIC_EXCEPTION);
  prod.event_cb(errorEvent);
  EXPECT_EQ(prod.getStats().ErrTopic, 3);
}

TEST_F(ProducerTest, DeliveryReportCbProcessesErrorsAndLogs) {
  ProducerStandIn prod{"nobroker", "notopic"};

  /// Create a logger factory which initializes the logger mock
  /// if this object goes out of scope, the logger mock is destroyed
  auto LoggerFactory = MockLoggerFactory();

  /// Register the Mocked logger to be chekced
  /// for called 8 times with the correct log
  REQUIRE_CALL(LoggerFactory.getMockedLogger(), log(_, _))
      .TIMES(8)
      .WITH(_1 == "KAFKA" &&
            _2.find("Rdkafka::Event::EVENT_ERROR") != std::string::npos);

  MockMessage errorMessage(RdKafka::ERR__TIMED_OUT);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrTimeout, 1);

  errorMessage = MockMessage(RdKafka::ERR_BROKER_NOT_AVAILABLE);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrBrokerNotAvailable, 1);

  errorMessage = MockMessage(RdKafka::ERR__TRANSPORT);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrTransport, 1);

  errorMessage = MockMessage(RdKafka::ERR__AUTHENTICATION);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrAuth, 1);

  errorMessage = MockMessage(RdKafka::ERR__MSG_TIMED_OUT);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrMsgTimeout, 1);

  errorMessage = MockMessage(RdKafka::ERR__UNKNOWN_TOPIC);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrTopic, 1);

  errorMessage = MockMessage(RdKafka::ERR_TOPIC_AUTHORIZATION_FAILED);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrTopic, 2);

  errorMessage = MockMessage(RdKafka::ERR_TOPIC_EXCEPTION);
  prod.dr_cb(errorMessage);
  EXPECT_EQ(prod.getStats().ErrTopic, 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
