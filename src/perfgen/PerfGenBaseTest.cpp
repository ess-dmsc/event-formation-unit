/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <perfgen/PerfGenBase.h>

class PerfGenBaseStandIn : public PerfGen::PerfGenBase {
public:
  PerfGenBaseStandIn(BaseSettings Settings, struct Multiblade::CAENSettings ReadoutSettings)
      : Perfgen::PerfGenBase(Settings, ReadoutSettings){};
  ~PerfGenBaseStandIn() = default;
  using Detector::Threads;
  using PerfGen::PerfGenBase::mystats;
};

class PerfGenBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  PerfGen::PerfGenSettings LocalSettings;
};

TEST_F(PerfGenBaseTest, Constructor) {
  PerfGenBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
