/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests delay line position calc code.
 */

#include "../AdcReadoutConstants.h"
#include "../DelayLinePositionCalc.h"
#include <cmath>
#include <gtest/gtest.h>

class ConstDelayLinePositionStandIn : public ConstDelayLinePosition {
public:
  ConstDelayLinePositionStandIn() = default;
  using ConstDelayLinePosition::getPosition;
};

TEST(ConstDelayLinePos, BasicTest) {
  auto ConstValue = 45465;
  ConstDelayLinePositionStandIn Pos;
  Pos.setCalibrationValues(ConstValue, 0.0);
  EXPECT_EQ(ConstValue, Pos.getPosition());
  EXPECT_TRUE(Pos.isValid());
}

class DelayLinePosInterfaceStandIn : public DelayLinePosCalcInterface {
public:
  DelayLinePosInterfaceStandIn(int Timeout)
      : DelayLinePosCalcInterface(Timeout){};
  using DelayLinePosCalcInterface::getPosition;
  using DelayLinePosCalcInterface::isValid;
  using DelayLinePosCalcInterface::reset;
  using DelayLinePosCalcInterface::getTimestamp;
  int getAmplitude() override { return 0; };
};

class DelayLinePosInterface : public ::testing::Test {
public:
  void SetUp() override {
    Tester = DelayLinePosInterfaceStandIn(TestTimeout);
    TestPulse = PulseParameters();
    TestPulse.PeakTimestamp = {1, 0};
    TestPulse.ThresholdTimestamp = {1, 0};
  }
  DelayLinePosInterfaceStandIn Tester{0};
  PulseParameters TestPulse;
  const std::uint64_t TestTimeout{300};
};

TEST_F(DelayLinePosInterface, GetPositionTest) {
  EXPECT_EQ(Tester.getPosition(), 0);
}

TEST_F(DelayLinePosInterface, ValidationFail1) {
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail3) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail4) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail5) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.ThresholdTimestamp = {0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail6) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.ThresholdTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.ThresholdTimestamp = {0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationFail7) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.ThresholdTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.ThresholdTimestamp = {1,
                                  static_cast<unsigned int>(TestTimeout + 1)};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationSuccess1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationSuccess2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.PeakTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ValidationSuccess3) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  auto ClockCycleLength = 1e9 / AdcSamplingRate;
  auto DiffClockCycles = static_cast<std::uint32_t>(
      static_cast<double>(TestTimeout) / ClockCycleLength);
  TestPulse.PeakTimestamp = {1, DiffClockCycles};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, ResetTest) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  auto ClockCycleLength = 1e9 / AdcSamplingRate;
  auto DiffClockCycles = static_cast<std::uint32_t>(
      static_cast<double>(TestTimeout) / ClockCycleLength);
  TestPulse.PeakTimestamp = {1, DiffClockCycles};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
  Tester.reset();
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLinePosInterface, GetTimestamp1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = {1, 0};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 1000000000u);
}

TEST_F(DelayLinePosInterface, GetTimestamp2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.ThresholdTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 1000000023u);
}

TEST_F(DelayLinePosInterface, GetTimestamp3) {
  Tester.setChannelRole({0, 0},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.ThresholdTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 1000000023u);
}

TEST_F(DelayLinePosInterface, GetTimestamp4) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 0u);
}

TEST_F(DelayLinePosInterface, GetTimestamp5) {
  Tester.setChannelRole({0, 0},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{1, 0};
  TestPulse.PeakTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 0u);
}

TEST_F(DelayLinePosInterface, GetTimestamp6) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{1, 0};
  TestPulse.PeakTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 0u);
}

TEST_F(DelayLinePosInterface, GetTimestamp7) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({1, 0},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{1, 0};
  TestPulse.PeakTimestamp = {1, 0};
  Tester.addPulse(TestPulse);

  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = {1, 1};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getTimestamp(), 1000000000u);
}

class DelayLineAmpPosCalcStandIn : public DelayLineAmpPosCalc {
public:
  DelayLineAmpPosCalcStandIn(int Timeout) : DelayLineAmpPosCalc(Timeout){};
  using DelayLineAmpPosCalc::getPosition;
  using DelayLineAmpPosCalc::isValid;
  using DelayLineAmpPosCalc::reset;
  using DelayLineAmpPosCalc::getTimestamp;
  using DelayLineAmpPosCalc::getAmplitude;
};

class DelayLineAmpCalc : public ::testing::Test {
public:
  void SetUp() override {
    Tester = DelayLineAmpPosCalcStandIn(TestTimeout);
    TestPulse = PulseParameters();
    TestPulse.PeakTimestamp = {1, 0};
    TestPulse.ThresholdTimestamp = {1, 0};
  }
  DelayLineAmpPosCalcStandIn Tester{0};
  PulseParameters TestPulse;
  const std::uint64_t TestTimeout{300};
};

TEST_F(DelayLineAmpCalc, ValidationSuccess1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLineAmpCalc, ValidationSuccess2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLineAmpCalc, ValidationSuccess3) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLineAmpCalc, ValidationFail1) {
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineAmpCalc, ValidationFail2) {
  Tester.setChannelRole({0, 2}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineAmpCalc, SingleEndCalc) {
  Tester.setChannelRole({0, 2}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 2};
  auto TestValue = 42;
  TestPulse.PeakAmplitude = TestValue;
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getPosition(), TestValue);
}

TEST_F(DelayLineAmpCalc, CalibrationTest1) {
  Tester.setChannelRole({0, 2}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 2};
  auto TestValue = 42;
  TestPulse.PeakAmplitude = TestValue;
  Tester.addPulse(TestPulse);
  auto CurrentPosition = TestValue;
  auto TestOffset = 5.5;
  auto TestSlope = 123.4;
  auto ExpectedNewValue = std::lround(TestOffset + CurrentPosition * TestSlope);
  Tester.setCalibrationValues(TestOffset, TestSlope);
  EXPECT_NEAR(Tester.getPosition(), ExpectedNewValue, 0.001);
}

TEST_F(DelayLineAmpCalc, CalibrationTest2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  auto TestValue = 42;
  TestPulse.PeakAmplitude = TestValue + 1;
  Tester.addPulse(TestPulse);

  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.PeakAmplitude = TestValue;
  Tester.addPulse(TestPulse);
  auto CurrentPosition = 1;
  auto TestOffset = 5.5;
  auto TestSlope = 123.4;
  auto ExpectedNewValue = std::lround(TestOffset + CurrentPosition * TestSlope);
  Tester.setCalibrationValues(TestOffset, TestSlope);
  EXPECT_NEAR(Tester.getPosition(), ExpectedNewValue, 0.001);
}

TEST_F(DelayLineAmpCalc, DoubleEndCalc) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  auto TestValue1 = 42;
  TestPulse.PeakAmplitude = TestValue1;
  Tester.addPulse(TestPulse);

  TestPulse.Identifier = ChannelID{0, 1};
  auto TestValue2 = 10;
  TestPulse.PeakAmplitude = TestValue2;
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getPosition(), std::lround(TestValue1 - TestValue2));
}

TEST_F(DelayLineAmpCalc, AmplitudeTest1) {
  Tester.setChannelRole({0, 2}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.PeakArea = 134324;
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getAmplitude(), 0);
}

TEST_F(DelayLineAmpCalc, AmplitudeTest2) {
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  auto TestAmplitude = 134324;
  TestPulse.PeakArea = TestAmplitude;
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getAmplitude(), TestAmplitude);
}

class DelayLineTimePosCalcStandIn : public DelayLineTimePosCalc {
public:
  DelayLineTimePosCalcStandIn(int Timeout) : DelayLineTimePosCalc(Timeout){};
  using DelayLineTimePosCalc::getPosition;
  using DelayLineTimePosCalc::isValid;
  using DelayLineTimePosCalc::reset;
  using DelayLineTimePosCalc::getTimestamp;
  using DelayLineTimePosCalc::getAmplitude;
};

class DelayLineTimeCalc : public ::testing::Test {
public:
  void SetUp() override {
    Tester = DelayLineTimePosCalcStandIn(TestTimeout);
    TestPulse = PulseParameters();
    TestPulse.PeakTimestamp = {1, 0};
    TestPulse.ThresholdTimestamp = {1, 0};
  }
  DelayLineTimePosCalcStandIn Tester{0};
  PulseParameters TestPulse;
  const std::uint64_t TestTimeout{300};
};

TEST_F(DelayLineTimeCalc, ValidationFailure1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, ValidationFailure2) {
  Tester.setChannelRole({0, 0},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, ValidationFailure3) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, ValidationFailure4) {
  Tester.setChannelRole({0, 0},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);

  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 1};
  Tester.addPulse(TestPulse);
  EXPECT_FALSE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, ValidationSucces1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);

  Tester.setChannelRole({0, 1},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 1};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, ValidationSucces2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  TestPulse.Identifier = ChannelID{0, 0};
  Tester.addPulse(TestPulse);

  Tester.setChannelRole({0, 1},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  TestPulse.Identifier = ChannelID{0, 1};
  Tester.addPulse(TestPulse);

  Tester.setChannelRole({0, 2}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  TestPulse.Identifier = ChannelID{0, 2};
  Tester.addPulse(TestPulse);
  EXPECT_TRUE(Tester.isValid());
}

TEST_F(DelayLineTimeCalc, SingleEndCalc) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  RawTimeStamp TimeStamp1{1, 50};
  RawTimeStamp TimeStamp2{1, 0};
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = TimeStamp1;
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.PeakTimestamp = TimeStamp2;
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getPosition(),
            static_cast<int>(TimeStamp1.GetTimeStampNS() -
                             TimeStamp2.GetTimeStampNS()));
}

TEST_F(DelayLineTimeCalc, CalibrationTest1) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  RawTimeStamp TimeStamp1{1, 50};
  RawTimeStamp TimeStamp2{1, 0};
  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = TimeStamp1;
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.PeakTimestamp = TimeStamp2;
  Tester.addPulse(TestPulse);
  auto CurrentPosition = Tester.getPosition();
  auto TestOffset = 5.5;
  auto TestSlope = 123.4;
  auto ExpectedNewValue = std::lround(TestOffset + CurrentPosition * TestSlope);
  Tester.setCalibrationValues(TestOffset, TestSlope);
  EXPECT_NEAR(Tester.getPosition(), ExpectedNewValue, 0.001);
}

TEST_F(DelayLineTimeCalc, CalibrationTest2) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  RawTimeStamp TimeStamp1{1, 0};
  RawTimeStamp TimeStamp2{1, 50};
  RawTimeStamp TimeStamp3{1, 50};

  TestPulse.Identifier = ChannelID{0, 2};
  TestPulse.PeakTimestamp = TimeStamp1;
  Tester.addPulse(TestPulse);

  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = TimeStamp2;
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.PeakTimestamp = TimeStamp3;
  Tester.addPulse(TestPulse);
  auto CurrentPosition = Tester.getPosition();
  auto TestOffset = 5.5;
  auto TestSlope = 123.4;
  auto ExpectedNewValue = std::lround(TestOffset + CurrentPosition * TestSlope);
  Tester.setCalibrationValues(TestOffset, TestSlope);
  EXPECT_NEAR(Tester.getPosition(), ExpectedNewValue, 0.001);
}

TEST_F(DelayLineTimeCalc, DoubleEndCalc) {
  Tester.setChannelRole({0, 0}, DelayLinePosCalcInterface::ChannelRole::FIRST);
  Tester.setChannelRole({0, 1}, DelayLinePosCalcInterface::ChannelRole::SECOND);
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  RawTimeStamp TimeStamp1{1, 10};
  RawTimeStamp TimeStamp2{1, 50};
  RawTimeStamp TimeStamp3{1, 55};

  TestPulse.Identifier = ChannelID{0, 2};
  TestPulse.PeakTimestamp = TimeStamp1;
  Tester.addPulse(TestPulse);

  TestPulse.Identifier = ChannelID{0, 0};
  TestPulse.PeakTimestamp = TimeStamp2;
  Tester.addPulse(TestPulse);
  TestPulse.Identifier = ChannelID{0, 1};
  TestPulse.PeakTimestamp = TimeStamp3;
  Tester.addPulse(TestPulse);
  auto TimeDiff1 = static_cast<int>(TimeStamp2.GetTimeStampNS() -
                                    TimeStamp1.GetTimeStampNS());
  auto TimeDiff2 = static_cast<int>(TimeStamp3.GetTimeStampNS() -
                                    TimeStamp1.GetTimeStampNS());
  auto ExpectedTimeDiff = std::lround((TimeDiff1 + TimeDiff2) / 2.0);
  EXPECT_EQ(Tester.getPosition(), ExpectedTimeDiff);
}

TEST_F(DelayLineTimeCalc, AmplitudeTest1) {
  Tester.setChannelRole({0, 2},
                        DelayLinePosCalcInterface::ChannelRole::REFERENCE);
  RawTimeStamp TimeStamp{1, 10};
  auto TestAmplitude = 213;
  TestPulse.Identifier = ChannelID{0, 2};
  TestPulse.PeakArea = TestAmplitude;
  Tester.addPulse(TestPulse);
  EXPECT_EQ(Tester.getAmplitude(), TestAmplitude);
}
