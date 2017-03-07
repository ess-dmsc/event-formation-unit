/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Time.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class TimeTest : public TestBase {
protected:
  Time *time;
  virtual void SetUp() { time = new Time(); }
  virtual void TearDown() { delete time; }
};

TEST_F(TimeTest, SettersGetters) {
  time->set_bc_clock(42.0);
  time->set_tac_slope(42.0);
  time->set_trigger_resolution(42.0);
  time->set_target_resolution(42.0);
  ASSERT_EQ(time->bc_clock(), 42.0);
  ASSERT_EQ(time->tac_slope(), 42.0);
  ASSERT_EQ(time->trigger_resolution(), 42.0);
  ASSERT_EQ(time->target_resolution(), 42.0);
}

TEST_F(TimeTest, Eval) {
  ASSERT_EQ(time->timestamp_ns(0, 0, 0), 0);
  ASSERT_EQ(time->timestamp(0, 0, 0), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
