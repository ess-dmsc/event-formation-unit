/** Copyright (C) 2016 European Spallation Source ERIC */


#include "TestBase.h"
#include <cspec/CalibrationFile.h>
#include <cspec/CSPECChanConv.h>
#include <string>

using namespace std;

extern int forcefstatfail;
extern int forcereadfail;
extern int forcewritefail;
extern int forceopenfail;

class CalibrationFileTest : public TestBase {
protected:

  virtual void SetUp() {  }
  virtual void TearDown() {  }
};

/** Test cases below */
TEST_F(CalibrationFileTest, LoadZeroAndMax) {
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  int res = calibfile.load(std::string("data/cal_zero"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, 0);

  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(0, wbuffer[i]);
    ASSERT_EQ(0, gbuffer[i]);
  }

  res = calibfile.load(std::string("data/cal_max"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, 0);

  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(65535, wbuffer[i]);
    ASSERT_EQ(65535, gbuffer[i]);
  }
}

TEST_F(CalibrationFileTest, SaveUninitialized) {
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  int res = calibfile.save(std::string("CalibrationFileTest"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, 0);
}

TEST_F(CalibrationFileTest, LoadFstatFail) {
  ASSERT_EQ(0, forcefstatfail);
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  forcefstatfail = 1;
  int res = calibfile.load(std::string("data/cal_zero"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, -1);
  ASSERT_EQ(0, forcefstatfail);
}

TEST_F(CalibrationFileTest, LoadReadFail) {
  ASSERT_EQ(0, forcereadfail);
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  forcereadfail = 1;
  int res = calibfile.load(std::string("data/cal_zero"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, -1);
  ASSERT_EQ(0, forcereadfail);
}

TEST_F(CalibrationFileTest, SaveWriteFail) {
  ASSERT_EQ(0, forcewritefail);
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  forcewritefail = 1;
  int res = calibfile.save(std::string("CalibrationFileTest"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, -1);
  ASSERT_EQ(0, forcewritefail);
}

TEST_F(CalibrationFileTest, SaveOpenFail) {
  ASSERT_EQ(0, forceopenfail);
  uint16_t wbuffer[CSPECChanConv::adcsize];
  uint16_t gbuffer[CSPECChanConv::adcsize];
  CalibrationFile calibfile;
  forceopenfail = 1;
  int res = calibfile.save(std::string("CalibrationFileTest"), (char*)wbuffer,
                    (char*)gbuffer);
  ASSERT_EQ(res, -1);
  ASSERT_EQ(0, forceopenfail);
}


int main(int argc, char **argv) {
  int __attribute__((unused)) ret = chdir("prototype2");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
