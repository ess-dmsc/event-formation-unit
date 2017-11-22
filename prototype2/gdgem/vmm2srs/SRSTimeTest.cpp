/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/SRSTime.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class SRSTimeTest : public TestBase {
protected:
  SRSTime *time;
  virtual void SetUp() { time = new SRSTime(); }
  virtual void TearDown() { delete time; }
};

TEST_F(SRSTimeTest, SettersGetters) {
  time->set_bc_clock(42.0);
  time->set_tac_slope(42.0);
  time->set_trigger_resolution(42.0);
  time->set_target_resolution(42.0);
  ASSERT_EQ(time->bc_clock(), 42.0);
  ASSERT_EQ(time->tac_slope(), 42.0);
  ASSERT_EQ(time->trigger_resolution(), 42.0);
  ASSERT_EQ(time->target_resolution(), 42.0);
}

TEST_F(SRSTimeTest, Eval) {
  ASSERT_EQ(time->timestamp_ns(0, 0, 0), 0);
  ASSERT_EQ(time->timestamp(0, 0, 0), 0);
}

TEST_F(SRSTimeTest, DebugString) {
  MESSAGE() << "This is not a test, just calling the debug function\n";
  auto debugstr = time->debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
