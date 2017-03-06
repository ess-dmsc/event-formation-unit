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

TEST_F(TimeTest, Eval) {
  ASSERT_EQ(time->timestamp_ns(0, 0, 0), 0);
  ASSERT_EQ(time->timestamp(0, 0, 0), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
