// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for KafkaConfig
///
//===----------------------------------------------------------------------===//

#include <common/kafka/KafkaConfig.h>
#include <common/testutils/TestBase.h>

class KafkaConfigTest : public TestBase {};

TEST_F(KafkaConfigTest, Default) {
  KafkaConfig KafkaCfg("");
  ASSERT_EQ(KafkaCfg.CfgParms.size(), 5);
}

TEST_F(KafkaConfigTest, LoadFileError) {
  ASSERT_THROW(KafkaConfig KafkaCfg("NoSuchFile"), std::runtime_error);
}

TEST_F(KafkaConfigTest, LoadFileOK) {
  KafkaConfig KafkaCfg(KAFKACONFIG_FILE);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
