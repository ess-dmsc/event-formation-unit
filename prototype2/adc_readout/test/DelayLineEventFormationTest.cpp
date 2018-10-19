/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../DelayLineEventFormation.h"
#include <gtest/gtest.h>

class DelayLineEventFormationStandIn : public DelayLineEventFormation {
public:
  DelayLineEventFormationStandIn(AdcSettings const &ReadoutSettings) : DelayLineEventFormation(ReadoutSettings) {};
  using DelayLineEventFormation::PulseHandlerMap;
  using DelayLineEventFormation::XAxisCalc;
  using DelayLineEventFormation::YAxisCalc;
};

class FormationOfEvents : public ::testing::Test {
public:
  void SetUp() override {
    DefaultSettings = AdcSettings{};
  };
  AdcSettings DefaultSettings;
};

TEST_F(FormationOfEvents, AxisInitTest1) {
  
}
