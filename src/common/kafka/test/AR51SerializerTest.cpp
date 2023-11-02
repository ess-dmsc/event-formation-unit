// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of AR51Serializer class
///
//===----------------------------------------------------------------------===//

#include <common/debug/Hexdump.h>
#include <common/kafka/AR51Serializer.h>
#include <common/kafka/Producer.h>
#include <common/testutils/TestBase.h>
#include <cstring>


struct MockProducer {
  inline void produce(nonstd::span<const uint8_t>, int64_t) { NumberOfCalls++; }

  size_t NumberOfCalls{0};
};

class AR51SerializerTest : public TestBase {
  void SetUp() override {
    for (int i = 0; i < 9000; i++) {
      RawData[i] = i % 27 + 64;
    }
  }

  void TearDown() override {}

protected:
  uint8_t RawData[9000];
  AR51Serializer ar52{"nameless"};
};


TEST_F(AR51SerializerTest, Serialize) {
  ASSERT_TRUE(ar52.FBuffer.empty());

  for (unsigned int i = 1; i <= 9000; i++) {
    auto buffer = ar52.serialize(RawData, i);
    ASSERT_TRUE(not ar52.FBuffer.empty());
    ASSERT_TRUE(buffer.size_bytes() > i + 54); // imperical value
    ASSERT_TRUE(buffer.size_bytes() < i + 68); // imperical value
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
