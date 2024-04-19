// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test of Frame1DHistogramSerializer class
///
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"
#include <common/kafka/serializer/Frame1DHistogramSerializer.h>
#include <common/testutils/TestBase.h>

struct MockProducer {
  inline void produce(nonstd::span<const uint8_t>, int64_t) { NumberOfCalls++; }

  size_t NumberOfCalls{0};
};

class Frame1DHistogramSerializerTest : public TestBase {
  void SetUp() override {}

  void TearDown() override {}

protected:
};

void function(nonstd::span<const uint8_t> data, int64_t timestamp) {
  std::cout << "Timestamp: " << timestamp << " Data size: " << data.size()
            << std::endl;
  uint size = 464;
  ASSERT_EQ(data.size(), size);
}

/// \todo THis is not a real test, just a placeholder. Build proper unit tests
/// here.
TEST_F(Frame1DHistogramSerializerTest, TestSerializer) {
  auto sender = serializer::Frame1DHistogramBuilder<int, double>(
      "some topic", 3333, 5, "intensity", "counts", "millisecond", function);

  std::map<int, int> data = {{10, 0}, {20, 1}, {30, 1}, {40, 0}, {50, 0},
                             {60, 1}, {70, 0}, {80, 0}, {90, 1}, {100, 0}};

  for (auto &e : data) {
    sender.addData(e.first, e.second);
  }

  sender.produce();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}