// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for KafkaConfig
///
//===----------------------------------------------------------------------===//

#include <common/kafka/KafkaConfig.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

std::string badkafkaconfigjson = R"(
  {
    "KafkaParms" : [
      {"message.max.bytes" : "10000000", "garbage" : true}
    ]
  }
)";

class KafkaConfigTest : public TestBase {};

TEST_F(KafkaConfigTest, Default) {
  KafkaConfig KafkaCfg("");
  ASSERT_EQ(KafkaCfg.CfgParms.size(), 8);
}

TEST_F(KafkaConfigTest, LoadFileError) {
  ASSERT_THROW(KafkaConfig KafkaCfg("NoSuchFile"), std::runtime_error);
}

TEST_F(KafkaConfigTest, BadKafkaConfigJson) {
  ASSERT_THROW(KafkaConfig KafkaCfg("NotKafkaConfig.json"), std::runtime_error);
}

TEST_F(KafkaConfigTest, LoadFileOK) { KafkaConfig KafkaCfg(KAFKACONFIG_FILE); }

int main(int argc, char **argv) {
  saveBuffer("NotKafkaConfig.json", (void *)badkafkaconfigjson.c_str(),
             badkafkaconfigjson.size());
  testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  deleteFile("NotKafkaConfig.json");
  return res;
}
