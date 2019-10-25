/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <jalousie/JalousieBase.h>
#include <test/TestBase.h>

class JalousieBaseStandIn : public Jalousie::JalousieBase {
public:
  JalousieBaseStandIn(BaseSettings Settings, struct Jalousie::CLISettings LocalJalousieSettings)
      : Jalousie::JalousieBase(Settings, LocalJalousieSettings){};
  ~JalousieBaseStandIn() = default;
  using Detector::Threads;
  using Jalousie::JalousieBase::mystats;
};

class JalousieBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  Jalousie::CLISettings LocalSettings;
};

TEST_F(JalousieBaseTest, Constructor) {
  JalousieBaseStandIn Jalousie(Settings, LocalSettings);
  EXPECT_EQ(Jalousie.mystats.rx_packets, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
