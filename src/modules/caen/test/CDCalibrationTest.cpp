// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <caen/geometry/CDCalibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

///
auto LokiExample = R"(
  {
    "Calibration":
    {
      "instrument" : "loki",
      "groups" : 2,
      "groupsize" : 7,
      "amplitudes" : 4,
      "pixellation" : 512,
      "default intervals" : [0.000, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],

      "Parameters" : [
        {
          "groupindex" : 0,
          "intervals"  : [0.0, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],
          "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]]
        },
        {
          "groupindex" : 1,
          "intervals"  : [0.0, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],
          "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]]
        }
      ]
    }
  }
)"_json;

using namespace Caen;

class CalibrationTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(CalibrationTest, Constructor) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  calib.parseCalibration();
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
