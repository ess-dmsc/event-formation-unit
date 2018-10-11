/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSTime.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

constexpr float no_offset = 0.0;
constexpr float unit_slope = 1.0;

class SRSTimeTest : public TestBase {
protected:
  SRSTime *time;
  virtual void SetUp() { time = new SRSTime(); }
  virtual void TearDown() { delete time; }
};

TEST_F(SRSTimeTest, SettersGetters) {
  time->set_bc_clock(42.0);
  time->set_tac_slope(42.0);
  time->set_trigger_resolution_ns(42.0);
  time->set_target_resolution_ns(42.0);
  time->set_acquisition_window(1234);
  ASSERT_EQ(time->bc_clock(), 42.0);
  ASSERT_EQ(time->tac_slope(), 42.0);
  ASSERT_EQ(time->trigger_resolution_ns(), 42.0);
  ASSERT_EQ(time->target_resolution_ns(), 42.0);
  ASSERT_EQ(time->acquisition_window(), 1234);
  ASSERT_EQ(time->internal_clock_period_ns(), 25);
}


TEST_F(SRSTimeTest, Eval) {
  ASSERT_EQ(time->target_resolution_ns(), 0.5);

  /// \todo should return 0.0 on zero input?
  MESSAGE() << "Warning - timestamp_ns() might not be what we want!\n";
  ASSERT_EQ(time->timestamp_ns(0, 0, 0, no_offset, unit_slope), 1);

  /// \todo I think this should actually return 0.5 ?
  MESSAGE() << "Warning - timestamp() might not be correct\n";
  ASSERT_EQ(time->timestamp(0, 0, 0, no_offset, unit_slope), 0);
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
