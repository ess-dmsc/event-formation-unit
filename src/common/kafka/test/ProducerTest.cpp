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
#include <fakeit.hpp>
#include <librdkafka/rdkafkacpp.h>

KafkaConfig KafkaCfg{""};

using fakeit::Mock;
using fakeit::When;
using testing::_;

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
protected:
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
  EXPECT_EQ(Stats.valueByName("kafka.stat_events"), 0);
  EXPECT_EQ(Stats.valueByName("kafka.error_events"), 0);
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

  /// will be deleted by the TestProducer when unique_ptr goes out of scope
  auto MockKafkaProducer = new MockProducer();

  RdKafka::ErrorCode ReturnValue = RdKafka::ERR__STATE;

  EXPECT_CALL(*MockKafkaProducer, produce(_, _, _, _, _, _, _, _, _))
      .Times(1)
      .WillRepeatedly(testing::Return(ReturnValue));

  EXPECT_CALL(*MockKafkaProducer, poll(_))
      .Times(1)
      .WillRepeatedly(testing::Return(0));

  prod.KafkaProducer.reset(MockKafkaProducer);
  uint8_t SomeData[20];

  ASSERT_EQ(prod.getStats().produce_errors, 0);

  int ret = prod.produce(SomeData, 999);

  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(prod.getStats().produce_errors, 1);
}

TEST_F(ProducerTest, ProducerSuccess) {
  ProducerStandIn TestProducer{"nobroker", "notopic"};

  /// will be deleted by the TestProducer when unique_ptr goes out of scope
  auto MockKafkaProducer = new MockProducer();

  RdKafka::ErrorCode ReturnValue = RdKafka::ERR_NO_ERROR;

  EXPECT_CALL(*MockKafkaProducer, produce(_, _, _, _, _, _, _, _, _))
      .Times(1)
      .WillRepeatedly(testing::Return(ReturnValue));

  EXPECT_CALL(*MockKafkaProducer, poll(_))
      .Times(1)
      .WillRepeatedly(testing::Return(0));

  TestProducer.KafkaProducer.reset(MockKafkaProducer);
  unsigned int NrOfBytes{200};
  auto SomeData = std::make_unique<unsigned char[]>(NrOfBytes);

  ASSERT_EQ(TestProducer.getStats().produce_errors, 0);
  ASSERT_EQ(TestProducer.getStats().produce_bytes_ok, 0);

  int ret = TestProducer.produce({SomeData.get(), NrOfBytes}, 999);
  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(TestProducer.getStats().produce_errors, 0);
  ASSERT_EQ(TestProducer.getStats().produce_bytes_ok, 200);
}

TEST_F(ProducerTest, ProducerFailDueToSize) {
  KafkaConfig KafkaCfg2("");
  ASSERT_EQ(KafkaCfg2.CfgParms.size(), static_cast<size_t>(6));
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

  Mock<RdKafka::Event> statsEvent;
  When(Method(statsEvent, type)).AlwaysReturn(RdKafka::Event::EVENT_STATS);
  When(Method(statsEvent, str)).AlwaysReturn(fakeEventJson);
  prod.event_cb(statsEvent.get());

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

  // Mock logger to capture log messages
  auto LoggerMock = MockLogger();
  Mock<RdKafka::Event> fakeEvent;
  When(Method(fakeEvent, type)).AlwaysReturn(RdKafka::Event::EVENT_ERROR);

  ON_CALL(LoggerMock, log(::testing::_, ::testing::_))
      .WillByDefault(
          [](const std::string &category, const std::string &message) {
            std::cout << "LoggerMock called with category: " << category
                      << ", message: " << message << std::endl;
          });

  // Testing for Timed out error
  EXPECT_CALL(LoggerMock,
              log("KAFKA", "Rdkafka error occurred: [-185] Local: Timed out"))
      .Times(1);

  When(Method(fakeEvent, err)).Return(RdKafka::ERR__TIMED_OUT);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrTimeout, 1);

  // Testing for Broker not available error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [8] Broker: Broker not available"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR_BROKER_NOT_AVAILABLE);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrBrokerNotAvailable, 1);

  // Testing for Broker transport failure error
  EXPECT_CALL(LoggerMock,
              log("KAFKA",
                  "Rdkafka error occurred: [-195] Local: Broker transport "
                  "failure"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR__TRANSPORT);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrTransport, 1);

  // Testing for Authentication failure error
  EXPECT_CALL(LoggerMock, log("KAFKA", "Rdkafka error occurred: [-169] "
                                       "Local: Authentication failure"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR__AUTHENTICATION);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrAuth, 1);

  // Testing for Message timed out error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [-192] Local: Message timed out"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR__MSG_TIMED_OUT);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrMsgTimeout, 1);

  // Testing for Unknown topic error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [-188] Local: Unknown topic"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR__UNKNOWN_TOPIC);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 1);

  // Testing for Topic authorization failed error
  EXPECT_CALL(LoggerMock,
              log("KAFKA", "Rdkafka error occurred: [29] Broker: Topic "
                           "authorization failed"))
      .Times(1);

  When(Method(fakeEvent, err))
      .AlwaysReturn(RdKafka::ERR_TOPIC_AUTHORIZATION_FAILED);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 2);

  // Testing for Invalid topic error
  EXPECT_CALL(LoggerMock,
              log("KAFKA", "Rdkafka error occurred: [17] Broker: Invalid topic"))
      .Times(1);

  When(Method(fakeEvent, err)).AlwaysReturn(RdKafka::ERR_TOPIC_EXCEPTION);
  prod.event_cb(fakeEvent.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 3);
}

TEST_F(ProducerTest, DeliveryReportCbProcessesErrorsAndLogs) {
  ProducerStandIn prod{"nobroker", "notopic"};

  // Mock logger to capture log messages
  auto LoggerMock = MockLogger();

  Mock<RdKafka::Message> fakeMessage;
  When(Method(fakeMessage, status))
      .AlwaysReturn(RdKafka::Message::MSG_STATUS_NOT_PERSISTED);

  ON_CALL(LoggerMock, log(::testing::_, ::testing::_))
      .WillByDefault(
          [](const std::string &category, const std::string &message) {
            std::cout << "LoggerMock called with category: " << category
                      << ", message: " << message << std::endl;
          });

  // Testing for Timed out error
  EXPECT_CALL(LoggerMock,
              log("KAFKA", "Rdkafka error occurred: [-185] Local: Timed out"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR__TIMED_OUT);

  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrTimeout, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 1);

  // Testing for Broker not available error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [8] Broker: Broker not available"))
      .Times(1);

  When(Method(fakeMessage, err))
      .AlwaysReturn(RdKafka::ERR_BROKER_NOT_AVAILABLE);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrBrokerNotAvailable, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 2);

  // Testing for Broker transport failure error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA",
          "Rdkafka error occurred: [-195] Local: Broker transport failure"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR__TRANSPORT);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrTransport, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 3);

  // Testing for Authentication failure error
  EXPECT_CALL(LoggerMock, log("KAFKA", "Rdkafka error occurred: [-169] "
                                       "Local: Authentication failure"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR__AUTHENTICATION);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrAuth, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 4);

  // Testing for Message timed out error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [-192] Local: Message timed out"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR__MSG_TIMED_OUT);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrMsgTimeout, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 5);

  // Testing for Unknown topic error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA", "Rdkafka error occurred: [-188] Local: Unknown topic"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR__UNKNOWN_TOPIC);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 6);

  // Testing for Topic authorization failed error
  EXPECT_CALL(
      LoggerMock,
      log("KAFKA",
          "Rdkafka error occurred: [29] Broker: Topic authorization failed"))
      .Times(1);

  When(Method(fakeMessage, err))
      .AlwaysReturn(RdKafka::ERR_TOPIC_AUTHORIZATION_FAILED);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 2);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 7);

  // Testing for Invalid topic error
  EXPECT_CALL(LoggerMock,
              log("KAFKA", "Rdkafka error occurred: [17] Broker: Invalid topic"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR_TOPIC_EXCEPTION);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrTopic, 3);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 8);

  // Testing for Message size too large error
  EXPECT_CALL(LoggerMock,
              log("KAFKA",
                  "Rdkafka error occurred: [10] Broker: Message size too large"))
      .Times(1);

  When(Method(fakeMessage, err)).AlwaysReturn(RdKafka::ERR_MSG_SIZE_TOO_LARGE);
  prod.dr_cb(fakeMessage.get());
  EXPECT_EQ(prod.getStats().ErrMsgSizeTooLarge, 1);
  EXPECT_EQ(prod.getStats().MsgStatusNotPersisted, 9);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}