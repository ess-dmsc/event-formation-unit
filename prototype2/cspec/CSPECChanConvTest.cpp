/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cspec/CSPECChanConv.h>
#include <test/TestBase.h>

using namespace std;

class CspecChanConvTest : public TestBase {
protected:
  CSPECChanConv conv;
};

/** Test cases below */

/** checking that arrays are cleared before use */
TEST_F(CspecChanConvTest, ConstructorDefault) {
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(conv.getwireid(i), i) << "wrong wire id conversion at adc value "
                                    << i << endl;
    ASSERT_EQ(conv.getgridid(i), i) << "wrong grid id conversion at adc value "
                                    << i << endl;
  }
}

TEST_F(CspecChanConvTest, ConstructorParametrized) {
  CSPECChanConv conv(7);
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(conv.getwireid(i), 7) << "wrong wire id conversion at adc value "
                                    << i << endl;
    ASSERT_EQ(conv.getgridid(i), 7) << "wrong grid id conversion at adc value "
                                    << i << endl;
  }
}

/** test invalid ranges and resolutions */
TEST_F(CspecChanConvTest, InvalidCalibrationParms) {

  int ret = conv.makewirecal(20000, 20500, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(2000, 500, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(2000, 2000, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(0, 127, 128);
  ASSERT_EQ(ret, -1);
}

/** Test boundaries of adc channel data */
TEST_F(CspecChanConvTest, GenerateCalibration) {
  int ret = conv.makewirecal(400, 2000, 128);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(conv.getwireid(0), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getwireid(399), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getwireid(400), 0);
  ASSERT_EQ(conv.getwireid(2000), 128);
  ASSERT_EQ(conv.getwireid(2001), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getwireid(CSPECChanConv::adcsize - 1),
            CSPECChanConv::adcsize - 1);
}

TEST_F(CspecChanConvTest, LoadCalibration) {
  uint16_t wirecal[CSPECChanConv::adcsize];
  uint16_t gridcal[CSPECChanConv::adcsize];
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    wirecal[i] = i;
    gridcal[i] = i + 10;
  }
  conv.load_calibration(wirecal, gridcal);
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(i, conv.getwireid(i));
    ASSERT_EQ(i + 10, conv.getgridid(i));
  }
  conv.load_calibration(0, 0);
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(i, conv.getwireid(i));
    ASSERT_EQ(i + 10, conv.getgridid(i));
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
