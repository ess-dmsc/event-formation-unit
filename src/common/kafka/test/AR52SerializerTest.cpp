// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of AR52Serializer class
///
//===----------------------------------------------------------------------===//

#include <common/debug/Hexdump.h>
#include <common/kafka/AR52Serializer.h>
#include <common/kafka/Producer.h>
#include <common/testutils/TestBase.h>
#include <cstring>


struct MockProducer {
  inline void produce(std::span<const uint8_t>, int64_t) { NumberOfCalls++; }

  size_t NumberOfCalls{0};
};

class AR52SerializerTest : public TestBase {
  void SetUp() override {
    for (int i = 0; i < 9000; i++) {
      RawData[i] = i % 27 + 64;
    }
  }

  void TearDown() override {}

protected:
  uint8_t RawData[9000];
  AR52Serializer ar52{"nameless"};
};

TEST_F(AR52SerializerTest, Serialize) {

  for (int i = 1; i < 10; i++) {
    auto buffer = ar52.serialize(RawData, i);
    hexDump(buffer.data(), buffer.size_bytes());
    printf("\n");
  }

  //EXPECT_GE(buffer.size_bytes(), ARRAYLENGTH * 8);
  //EXPECT_LE(buffer.size_bytes(), ARRAYLENGTH * 8 + 128);
  //ASSERT_TRUE(not buffer.empty());

  //EXPECT_EQ(std::string(reinterpret_cast<const char *>(&buffer[4]), 4), "ar52");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
