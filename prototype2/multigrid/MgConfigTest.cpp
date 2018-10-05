/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/DataSave.h>
#include <cstring>
#include <multigrid/MgConfig.h>
#include <memory>
#include <test/TestBase.h>
#include <unistd.h>

std::string pathprefix{""};

std::string validconfig = R"(
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

class MGConfigTest : public TestBase {
protected:
  virtual void SetUp() { }
  virtual void TearDown() { }
};

/** Test cases below */
TEST_F(MGConfigTest, Constructor) {
  Multigrid::Config mgconfig;
  ASSERT_FALSE(mgconfig.spoof_high_time);
  ASSERT_EQ(mgconfig.reduction_strategy, "");
  ASSERT_EQ(mgconfig.mappings.max_z(), 0);
}

TEST_F(MGConfigTest, ValidConfig) {
  std::string filename{"deleteme_mgvalidconfig.json"};
  DataSave tempfile(filename, (void *)validconfig.c_str(), validconfig.size());
  Multigrid::Config mgconfig(filename);
  ASSERT_TRUE(mgconfig.spoof_high_time);
  ASSERT_EQ(mgconfig.reduction_strategy, "maximum");
  ASSERT_EQ(mgconfig.mappings.max_z(), 20);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
