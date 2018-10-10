/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>
#include <common/DataSave.h>
#include <multigrid/MGMesytecBase.h>
#include <multigrid/mgmesytec/TestData.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

std::string mgconfigjson = R"(
  {
    "reduction_strategy" : "maximum",

    "spoof_high_time" : true,

    "geometry_mappings" :
    [
      {"max_channel":120, "max_wire":80, "max_z":20, "swap_wires":true, "swap_grids":false, "flipped_x":true, "flipped_z":true},
      {"max_channel":120, "max_wire":80, "max_z":20, "swap_wires":true, "swap_grids":false, "flipped_x":true, "flipped_z":true,
        "grid_filters":{
          "blanket":{"min":0, "max":65535, "rescale":1.0},
          "exceptions":[
            {"idx":26, "rescale":1.5},
            {"idx":28, "rescale":1.5},
            {"idx":29, "rescale":0.5},
            {"idx":33, "rescale":1.5},
            {"idx":34, "rescale":1.5}
          ]
        }
      },
      {"max_channel":120, "max_wire":80, "max_z":20, "swap_wires":true, "swap_grids":false, "flipped_x":true, "flipped_z":true}
    ]
  }
)";

class MGMesytecBaseStandIn : public MGMesytecBase {
public:
  MGMesytecBaseStandIn(BaseSettings Settings, struct MGMesytecSettings ReadoutSettings)
      : MGMesytecBase(Settings, ReadoutSettings){};
  ~MGMesytecBaseStandIn() = default;
  using Detector::Threads;
  using MGMesytecBase::mystats;
};

class MGMesytecBaseTest : public ::testing::Test {
public:
  virtual void SetUp() {
    LocalSettings.ConfigFile = "mgconfig.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.MinimumMTU = 1500;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  MGMesytecSettings LocalSettings;
};

TEST_F(MGMesytecBaseTest, Constructor) {
  MGMesytecBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.triggers, 0);
  EXPECT_EQ(Readout.mystats.readouts, 0);
}


TEST_F(MGMesytecBaseTest, DataReceive) {
  MGMesytecBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&ws2_short[0], ws2_short.size());
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, ws2_short.size());
  EXPECT_EQ(Readout.mystats.readouts, 34); // 
  //EXPECT_EQ(Readout.mystats.events, 2);
  EXPECT_EQ(Readout.mystats.bus_glitches, 0);
}

int main(int argc, char **argv) {
  std::string filename{"mgconfig.json"};
  DataSave tempfile(filename, (void *)mgconfigjson.c_str(), mgconfigjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
