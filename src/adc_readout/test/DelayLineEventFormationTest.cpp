/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../DelayLineEventFormation.h"
#include <common/Log.h>
#include <gtest/gtest.h>
#include <trompeloeil.hpp>

class DelayLineEventFormationStandIn : public DelayLineEventFormation {
public:
  explicit DelayLineEventFormationStandIn(AdcSettings const &ReadoutSettings)
      : DelayLineEventFormation(ReadoutSettings){};
  using DelayLineEventFormation::DoChannelRoleMapping;
  using DelayLineEventFormation::PulseHandlerMap;
  using DelayLineEventFormation::XAxisCalc;
  using DelayLineEventFormation::YAxisCalc;
};

class FormationOfEventsInit : public ::testing::Test {
public:
  void SetUp() override {
    Log::SetMinimumSeverity(Log::Severity::Critical);
    DefaultSettings = AdcSettings{};
  };
  void TearDown() override { Log::SetMinimumSeverity(Log::Severity::Error); };
  AdcSettings DefaultSettings;
};

TEST_F(FormationOfEventsInit, AxisInitTest1) {
  using AxisType = AdcSettings::PositionSensingType;
  DefaultSettings.XAxis = AxisType::AMPLITUDE;
  DefaultSettings.YAxis = AxisType::TIME;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(dynamic_cast<DelayLineTimePosCalc *>(Tester.XAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<DelayLineAmpPosCalc *>(Tester.XAxisCalc.get()),
            nullptr);

  EXPECT_EQ(dynamic_cast<DelayLineAmpPosCalc *>(Tester.YAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<DelayLineTimePosCalc *>(Tester.YAxisCalc.get()),
            nullptr);
}

TEST_F(FormationOfEventsInit, AxisInitTest2) {
  using AxisType = AdcSettings::PositionSensingType;
  DefaultSettings.XAxis = AxisType::TIME;
  DefaultSettings.YAxis = AxisType::AMPLITUDE;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(dynamic_cast<DelayLineAmpPosCalc *>(Tester.XAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<DelayLineTimePosCalc *>(Tester.XAxisCalc.get()),
            nullptr);

  EXPECT_EQ(dynamic_cast<DelayLineTimePosCalc *>(Tester.YAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<DelayLineAmpPosCalc *>(Tester.YAxisCalc.get()),
            nullptr);
}

TEST_F(FormationOfEventsInit, AxisInitTest3) {
  using AxisType = AdcSettings::PositionSensingType;
  DefaultSettings.XAxis = AxisType::CONST;
  DefaultSettings.YAxis = AxisType(42);
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(dynamic_cast<DelayLineAmpPosCalc *>(Tester.XAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<ConstDelayLinePosition *>(Tester.XAxisCalc.get()),
            nullptr);

  EXPECT_EQ(dynamic_cast<DelayLineTimePosCalc *>(Tester.YAxisCalc.get()),
            nullptr);
  EXPECT_NE(dynamic_cast<ConstDelayLinePosition *>(Tester.YAxisCalc.get()),
            nullptr);
}

class DelayLineTestClass : public DelayLinePositionInterface {
public:
  DelayLineTestClass() = default;
  using DelayLinePositionInterface::Origin;
  using DelayLinePositionInterface::Slope;
};

TEST_F(FormationOfEventsInit, CalibrationTest) {
  using AxisType = AdcSettings::PositionSensingType;
  DefaultSettings.XAxis = AxisType::CONST;
  DefaultSettings.YAxis = AxisType(42);
  DefaultSettings.XAxisCalibOffset = -456;
  DefaultSettings.XAxisCalibSlope = 1234.67;
  DefaultSettings.YAxisCalibOffset = 42.123;
  DefaultSettings.YAxisCalibSlope = 987.654;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  auto XAxisCalib = Tester.XAxisCalc->getCalibrationValues();
  auto YAxisCalib = Tester.YAxisCalc->getCalibrationValues();
  EXPECT_EQ(XAxisCalib.Origin, DefaultSettings.XAxisCalibOffset);
  EXPECT_EQ(XAxisCalib.Slope, DefaultSettings.XAxisCalibSlope);

  EXPECT_EQ(YAxisCalib.Origin, DefaultSettings.YAxisCalibOffset);
  EXPECT_EQ(YAxisCalib.Slope, DefaultSettings.YAxisCalibSlope);
}

TEST_F(FormationOfEventsInit, ChannelInitNoRoles) {
  using AxisType = AdcSettings::PositionSensingType;
  DefaultSettings.XAxis = AxisType::AMPLITUDE;
  DefaultSettings.YAxis = AxisType::AMPLITUDE;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_TRUE(Tester.PulseHandlerMap.empty());
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannels1) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::AMPLITUDE;
  DefaultSettings.YAxis = AxisType::AMPLITUDE;
  DefaultSettings.ADC1Channel1 = ChRole::AMPLITUDE_X_AXIS_1;
  DefaultSettings.ADC1Channel2 = ChRole::AMPLITUDE_X_AXIS_2;
  DefaultSettings.ADC1Channel3 = ChRole::AMPLITUDE_Y_AXIS_1;
  DefaultSettings.ADC1Channel4 = ChRole::AMPLITUDE_Y_AXIS_2;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(Tester.PulseHandlerMap.size(), 4u);
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannels2) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::AMPLITUDE;
  DefaultSettings.YAxis = AxisType::AMPLITUDE;
  DefaultSettings.ADC1Channel1 = ChRole::AMPLITUDE_X_AXIS_1;
  DefaultSettings.ADC1Channel2 = ChRole::AMPLITUDE_X_AXIS_2;
  DefaultSettings.ADC1Channel3 = ChRole::AMPLITUDE_X_AXIS_1;
  DefaultSettings.ADC1Channel4 = ChRole::AMPLITUDE_X_AXIS_2;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(Tester.PulseHandlerMap.size(), 4u);
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannels3) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::TIME;
  DefaultSettings.YAxis = AxisType::TIME;
  DefaultSettings.ADC1Channel1 = ChRole::TIME_X_AXIS_1;
  DefaultSettings.ADC1Channel2 = ChRole::TIME_X_AXIS_2;
  DefaultSettings.ADC1Channel3 = ChRole::TIME_Y_AXIS_1;
  DefaultSettings.ADC1Channel4 = ChRole::TIME_Y_AXIS_2;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(Tester.PulseHandlerMap.size(), 4u);
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannelsFailure1) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::AMPLITUDE;
  DefaultSettings.YAxis = AxisType::AMPLITUDE;
  DefaultSettings.ADC1Channel1 = ChRole::TIME_X_AXIS_1;
  DefaultSettings.ADC1Channel2 = ChRole::TIME_X_AXIS_2;
  DefaultSettings.ADC1Channel3 = ChRole::TIME_Y_AXIS_1;
  DefaultSettings.ADC1Channel4 = ChRole::TIME_Y_AXIS_2;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_TRUE(Tester.PulseHandlerMap.empty());
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannelsFailure2) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::TIME;
  DefaultSettings.YAxis = AxisType::TIME;
  DefaultSettings.ADC1Channel1 = ChRole::AMPLITUDE_X_AXIS_1;
  DefaultSettings.ADC1Channel2 = ChRole::AMPLITUDE_X_AXIS_2;
  DefaultSettings.ADC1Channel3 = ChRole::AMPLITUDE_Y_AXIS_1;
  DefaultSettings.ADC1Channel4 = ChRole::AMPLITUDE_Y_AXIS_2;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_TRUE(Tester.PulseHandlerMap.empty());
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannelsFailure3) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::CONST;
  DefaultSettings.YAxis = AxisType::CONST;
  DefaultSettings.ADC1Channel1 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel2 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel3 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel4 = ChRole::REFERENCE_TIME;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_TRUE(Tester.PulseHandlerMap.empty());
}

TEST_F(FormationOfEventsInit, ChannelInitAllChannels4) {
  using AxisType = AdcSettings::PositionSensingType;
  using ChRole = AdcSettings::ChannelRole;
  DefaultSettings.XAxis = AxisType::TIME;
  DefaultSettings.YAxis = AxisType::CONST;
  DefaultSettings.ADC1Channel1 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel2 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel3 = ChRole::REFERENCE_TIME;
  DefaultSettings.ADC1Channel4 = ChRole::REFERENCE_TIME;
  DelayLineEventFormationStandIn Tester(DefaultSettings);
  EXPECT_EQ(Tester.PulseHandlerMap.size(), 4u);
}

using trompeloeil::_;

class MockDelayLineAxis : public DelayLinePositionInterface {
public:
  MockDelayLineAxis() = default;
  MAKE_MOCK0(isValid, bool(), override);
  MAKE_MOCK0(getPosition, int(), override);
  MAKE_MOCK0(getAmplitude, int(), override);
  MAKE_MOCK0(getTimestamp, std::uint64_t(), override);
};

class FormationOfEventsValid : public ::testing::Test {
public:
  void SetUp() override {
    DefaultSettings = AdcSettings{};
    using AxisType = AdcSettings::PositionSensingType;
    DefaultSettings.XAxis = AxisType::CONST;
    DefaultSettings.YAxis = AxisType::CONST;
    UnderTest = DelayLineEventFormationStandIn(DefaultSettings);
    UnderTest.XAxisCalc = std::make_unique<MockDelayLineAxis>();
    UnderTest.YAxisCalc = std::make_unique<MockDelayLineAxis>();
  };
  AdcSettings DefaultSettings;
  DelayLineEventFormationStandIn UnderTest{DefaultSettings};
};

TEST_F(FormationOfEventsValid, ValidTestSuccess) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.XAxisCalc.get()),
               isValid())
      .TIMES(1)
      .RETURN(true);
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.YAxisCalc.get()),
               isValid())
      .TIMES(1)
      .RETURN(true);
  EXPECT_TRUE(UnderTest.hasValidEvent());
}

TEST_F(FormationOfEventsValid, ValidTestFail1) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.XAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(false);
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.YAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(true);
  EXPECT_FALSE(UnderTest.hasValidEvent());
}

TEST_F(FormationOfEventsValid, ValidTestFail2) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.XAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(true);
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.YAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(false);
  EXPECT_FALSE(UnderTest.hasValidEvent());
}

TEST_F(FormationOfEventsValid, ValidTestFail3) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.XAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(false);
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAxis *>(UnderTest.YAxisCalc.get()),
               isValid())
      .TIMES(0, 1)
      .RETURN(false);
  EXPECT_FALSE(UnderTest.hasValidEvent());
}

class MockDelayLineAmpAxis : public DelayLineAmpPosCalc {
public:
  MockDelayLineAmpAxis() : DelayLineAmpPosCalc(0){};
  MAKE_MOCK0(isValid, bool(), override);
  MAKE_MOCK0(getPosition, int(), override);
  MAKE_MOCK0(getAmplitude, int(), override);
  MAKE_MOCK0(getTimestamp, std::uint64_t(), override);
  MAKE_MOCK1(addPulse, void(PulseParameters const &), override);
  MAKE_MOCK0(popEvent, AxisEvent(), override);
};

class MockDelayLineTimeAxis : public DelayLineTimePosCalc {
public:
  MockDelayLineTimeAxis() : DelayLineTimePosCalc(0){};
  MAKE_MOCK0(isValid, bool(), override);
  MAKE_MOCK0(getPosition, int(), override);
  MAKE_MOCK0(getAmplitude, int(), override);
  MAKE_MOCK0(getTimestamp, std::uint64_t(), override);
  MAKE_MOCK1(addPulse, void(PulseParameters const &), override);
  MAKE_MOCK0(popEvent, AxisEvent(), override);
};

class FormationOfEvents : public ::testing::Test {
public:
  void SetUp() override {
    DefaultSettings = AdcSettings{};
    using AxisType = AdcSettings::PositionSensingType;
    DefaultSettings.XAxis = AxisType::AMPLITUDE;
    DefaultSettings.YAxis = AxisType::TIME;
    DefaultSettings.ADC1Channel1 = AdcSettings::ChannelRole::AMPLITUDE_X_AXIS_1;
    DefaultSettings.ADC1Channel2 = AdcSettings::ChannelRole::REFERENCE_TIME;
    DefaultSettings.ADC1Channel3 = AdcSettings::ChannelRole::TIME_Y_AXIS_1;
    UnderTest = DelayLineEventFormationStandIn(DefaultSettings);
    auto TempXAxisPtr = UnderTest.XAxisCalc.get();
    auto TempYAxisPtr = UnderTest.YAxisCalc.get();
    UnderTest.XAxisCalc = std::make_unique<MockDelayLineAmpAxis>();
    UnderTest.YAxisCalc = std::make_unique<MockDelayLineTimeAxis>();
    for (auto &Handler : UnderTest.PulseHandlerMap) {
      if (Handler.second == TempXAxisPtr) {
        Handler.second = UnderTest.XAxisCalc.get();
      } else if (Handler.second == TempYAxisPtr) {
        Handler.second = UnderTest.YAxisCalc.get();
      }
    }
  };
  AdcSettings DefaultSettings;
  DelayLineEventFormationStandIn UnderTest{DefaultSettings};
};

TEST_F(FormationOfEvents, addPulseSuccess1) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get()),
               addPulse(_))
      .TIMES(0); /// \todo Should probably not be 0.
  PulseParameters TestPulse;
  TestPulse.Identifier = {0, 0};
  UnderTest.addPulse(TestPulse);
  EXPECT_EQ(UnderTest.getNrOfDiscardedPulses(), 0u);
  EXPECT_EQ(UnderTest.getNrOfProcessedPulses(), 1u);
}

TEST_F(FormationOfEvents, addPulseSuccess2) {
  REQUIRE_CALL(
      *dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get()),
      addPulse(_))
      .TIMES(0); /// \todo Should probably not be 0.
  PulseParameters TestPulse;
  TestPulse.Identifier = {0, 2};
  UnderTest.addPulse(TestPulse);
  EXPECT_EQ(UnderTest.getNrOfDiscardedPulses(), 0u);
  EXPECT_EQ(UnderTest.getNrOfProcessedPulses(), 1u);
}

TEST_F(FormationOfEvents, addPulseSuccess3) {
  REQUIRE_CALL(*dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get()),
               addPulse(_))
      .TIMES(0); /// \todo Should probably not be 0.
  REQUIRE_CALL(
      *dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get()),
      addPulse(_))
      .TIMES(0); /// \todo Should probably not be 0.
  PulseParameters TestPulse;
  TestPulse.Identifier = {0, 1};
  UnderTest.addPulse(TestPulse);
  EXPECT_EQ(UnderTest.getNrOfDiscardedPulses(), 0u);
  EXPECT_EQ(UnderTest.getNrOfProcessedPulses(), 1u);
}

TEST_F(FormationOfEvents, addPulseFailure) {
  FORBID_CALL(*dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get()),
              addPulse(_));
  FORBID_CALL(*dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get()),
              addPulse(_));
  PulseParameters TestPulse;
  TestPulse.Identifier = {0, 3};
  UnderTest.addPulse(TestPulse);
  EXPECT_EQ(UnderTest.getNrOfDiscardedPulses(), 1u);
  EXPECT_EQ(UnderTest.getNrOfProcessedPulses(), 0u);
}

TEST_F(FormationOfEvents, PopEventSuccess) {
  auto XAxisPtr =
      dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get());
  auto YAxisPtr =
      dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get());
  REQUIRE_CALL(*XAxisPtr, isValid()).TIMES(1).RETURN(true);
  REQUIRE_CALL(*YAxisPtr, isValid()).TIMES(1).RETURN(true);

  REQUIRE_CALL(*XAxisPtr, popEvent()).TIMES(1).RETURN(AxisEvent{});
  REQUIRE_CALL(*YAxisPtr, popEvent()).TIMES(1).RETURN(AxisEvent{});
  UnderTest.popEvent();
}

TEST_F(FormationOfEvents, PopEventFailure1) {
  auto XAxisPtr =
      dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get());
  auto YAxisPtr =
      dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get());
  ALLOW_CALL(*XAxisPtr, isValid()).RETURN(false);
  ALLOW_CALL(*YAxisPtr, isValid()).RETURN(false);
  UnderTest.popEvent();
}

TEST_F(FormationOfEvents, PopEventFailure2) {
  auto XAxisPtr =
      dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get());
  auto YAxisPtr =
      dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get());
  ALLOW_CALL(*XAxisPtr, isValid()).RETURN(true);
  ALLOW_CALL(*YAxisPtr, isValid()).RETURN(false);
  UnderTest.popEvent();
}

TEST_F(FormationOfEvents, PopEventFailure3) {
  auto XAxisPtr =
      dynamic_cast<MockDelayLineAmpAxis *>(UnderTest.XAxisCalc.get());
  auto YAxisPtr =
      dynamic_cast<MockDelayLineTimeAxis *>(UnderTest.YAxisCalc.get());
  ALLOW_CALL(*XAxisPtr, isValid()).RETURN(false);
  ALLOW_CALL(*YAxisPtr, isValid()).RETURN(true);
  UnderTest.popEvent();
}
