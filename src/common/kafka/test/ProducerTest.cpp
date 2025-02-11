// Copyright (C) 2016-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Unit test for Kafka Producer
//===----------------------------------------------------------------------===//

#include "KafkaMocks.h"
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
  ProducerStandIn(const std::string &Broker, const std::string &Topic)
      : Producer(Broker, Topic, KafkaCfg.CfgParms) {}
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
  ASSERT_EQ(prod.ProducerStatCounters.dr_errors, 0);
  ASSERT_EQ(prod.ProducerStatCounters.dr_noerrors, 0);
  ASSERT_EQ(prod.ProducerStatCounters.ev_errors, 0);
  // ASSERT_EQ(prod.stats.ev_others, 0);
  ASSERT_EQ(prod.ProducerStatCounters.produce_errors, 0);
}

TEST_F(ProducerTest, ConfigError) {
  ProducerStandIn prod{"nobroker", "notopic"};
  auto Res = prod.setConfig("queue.buffering.max.ms", "101");
  ASSERT_EQ(Res, RdKafka::Conf::CONF_OK);
  ASSERT_EQ(prod.ProducerStatCounters.config_errors, 0);

  Res = prod.setConfig("this.does.not.exist", "so.this.has.no.meaning");
  ASSERT_NE(Res, RdKafka::Conf::CONF_OK);
  ASSERT_EQ(prod.ProducerStatCounters.config_errors, 1);
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
  ASSERT_EQ(prod.ProducerStatCounters.produce_errors, 0);
  int ret = prod.produce(SomeData, 999);
  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(prod.ProducerStatCounters.produce_errors, 1);
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
  ASSERT_EQ(prod.ProducerStatCounters.produce_errors, 0);
  int ret = prod.produce({SomeData.get(), NrOfBytes}, 999);
  ASSERT_EQ(ret, ReturnValue);
  ASSERT_EQ(prod.ProducerStatCounters.produce_errors, 0);
}

TEST_F(ProducerTest, ProducerFailDueToSize) {
  KafkaConfig KafkaCfg2("");
  ASSERT_EQ(KafkaCfg2.CfgParms.size(), 5);
  Producer prod{"nobroker", "notopic", KafkaCfg2.CfgParms};
  auto DataPtr = std::make_unique<unsigned char>();
  int ret = prod.produce({DataPtr.get(), 100000000}, 1);
  ASSERT_EQ(ret, RdKafka::ERR_MSG_SIZE_TOO_LARGE);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
