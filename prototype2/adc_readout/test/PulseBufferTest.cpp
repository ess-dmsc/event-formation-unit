/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../PulseBuffer.h"
#include <gtest/gtest.h>

PulseParameters createPulse(ChannelID ID, std::int64_t Timestamp) {
  PulseParameters Pulse;
  Pulse.Identifier = ID;
  Pulse.ThresholdTimestampNS = Timestamp;
  return Pulse;
}

class PulseBufferTest : public ::testing::Test {
public:
  void SetUp() override {}
  const std::uint64_t Timeout = 1000;
  PulseBuffer TestBuffer{Timeout};
  std::uint64_t BaseTimeStamp{123456789};
};

TEST_F(PulseBufferTest, EnoughPulses0) {
  EXPECT_TRUE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, EnoughPulses1) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, EnoughPulses2) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({0, 1});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({0, 1}, BaseTimeStamp));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, NotEnoughPulses0) {
  TestBuffer.addChannel({0, 0});
  EXPECT_FALSE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, NotEnoughPulses1) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({0, 1});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp));
  EXPECT_FALSE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, NotEnoughPulses2) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({0, 1});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({0, 2}, BaseTimeStamp));
  EXPECT_FALSE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, NotEnoughPulses3) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  EXPECT_FALSE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, InvalidPulses1) {
  EXPECT_EQ(0, TestBuffer.getInvalidPulses());
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  EXPECT_EQ(1, TestBuffer.getInvalidPulses());
}

TEST_F(PulseBufferTest, InvalidPulses2) {
  EXPECT_EQ(0, TestBuffer.getInvalidPulses());
  TestBuffer.addChannel({0, 0});
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  EXPECT_EQ(1, TestBuffer.getInvalidPulses());
}

TEST_F(PulseBufferTest, WithinTimeout1) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, WithinTimeout2) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + 100));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, OutsideTimeout1) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 2));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  EXPECT_FALSE(TestBuffer.hasValidPulses());
}

TEST_F(PulseBufferTest, OutsideTimeout2) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 2));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp + Timeout * 4));
  EXPECT_FALSE(TestBuffer.hasValidPulses());
  EXPECT_EQ(TestBuffer.getDiscardedPulses(), 2);
}

TEST_F(PulseBufferTest, WithinTimeout3) {
  EXPECT_EQ(TestBuffer.getDiscardedPulses(), 0);
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 2));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp + Timeout * 2.5));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
  EXPECT_EQ(TestBuffer.getDiscardedPulses(), 1);
}

TEST_F(PulseBufferTest, WithinTimeout4) {
  EXPECT_EQ(TestBuffer.getDiscardedPulses(), 0);
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 2));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp + Timeout * 5));
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 5.5));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
  EXPECT_EQ(TestBuffer.getDiscardedPulses(), 2);
}

TEST_F(PulseBufferTest, ExpectedPulses) {
  TestBuffer.addChannel({0, 0});
  TestBuffer.addChannel({1, 0});
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 2));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp));
  TestBuffer.addPulse(createPulse({1, 0}, BaseTimeStamp + Timeout * 5));
  TestBuffer.addPulse(createPulse({0, 0}, BaseTimeStamp + Timeout * 5.5));
  EXPECT_TRUE(TestBuffer.hasValidPulses());
  auto Result = TestBuffer.getPulses();
  std::sort(Result.begin(), Result.end(), [](auto A, auto B) {
    return A.ThresholdTimestampNS > B.ThresholdTimestampNS;
  });
  ASSERT_EQ(Result.size(), 2u);
  EXPECT_EQ(Result[0].ThresholdTimestampNS, BaseTimeStamp + Timeout * 5.5);
  EXPECT_EQ(Result[1].ThresholdTimestampNS, BaseTimeStamp + Timeout * 5);
}
