/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSTime.h>
#include <string>
#include <common/testutils/TestBase.h>
#include <unistd.h>

using namespace Gem;

class SRSTimeTest : public TestBase {
protected:
  SRSTime time;
};

TEST_F(SRSTimeTest, SettersGetters) {
  time.bc_clock_MHz(42.0);
  time.tac_slope_ns(21.0);
  time.trigger_resolution_ns(7.0);
  time.acquisition_window(1234);
  EXPECT_EQ(time.bc_clock_MHz(), 42.0);
  EXPECT_EQ(time.tac_slope_ns(), 21.0);
  EXPECT_EQ(time.trigger_resolution_ns(), 7.0);
  EXPECT_EQ(time.acquisition_window(), 1234);
}


TEST_F(SRSTimeTest, Eval) {

  /// \todo should return 0.0 on zero input?
//  MESSAGE() << "Warning - timestamp_ns() might not be what we want!\n";
//  EXPECT_EQ(time.timestamp_ns(0, 0, 0, no_offset, unit_slope), 1);

}

TEST_F(SRSTimeTest, DebugString) {
  MESSAGE() << "This is not a test, just calling the debug function\n";
  auto debugstr = time.debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
