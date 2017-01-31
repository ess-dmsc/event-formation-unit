/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "TestBase.h"
#include <cspec/CSPECEvent.h>

using namespace std;

class CSPECEventTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/** Test cases below */
TEST_F(CSPECEventTest, Constructor) {
  CSPECEvent event(0xffeeddccbbaa9988, 0xffeeddcc);
  ASSERT_EQ(0x88, event.gettimestamp() >> 0 & 0xff);
  ASSERT_EQ(0x99, event.gettimestamp() >> 8 & 0xff);
  ASSERT_EQ(0xaa, event.gettimestamp() >> 16 & 0xff);
  ASSERT_EQ(0xbb, event.gettimestamp() >> 24 & 0xff);
  ASSERT_EQ(0xcc, event.gettimestamp() >> 32 & 0xff);
  ASSERT_EQ(0xdd, event.gettimestamp() >> 40 & 0xff);
  ASSERT_EQ(0xee, event.gettimestamp() >> 48 & 0xff);
  ASSERT_EQ(0xff, event.gettimestamp() >> 56 & 0xff);

  ASSERT_EQ(0xcc, event.getpixelid() >> 0 & 0xff);
  ASSERT_EQ(0xdd, event.getpixelid() >> 8 & 0xff);
  ASSERT_EQ(0xee, event.getpixelid() >> 16 & 0xff);
  ASSERT_EQ(0xff, event.getpixelid() >> 24 & 0xff);
}

TEST_F(CSPECEventTest, OperatorLessAndGreaterThan) {
  CSPECEvent event1(0xdeadbeef, 100);
  CSPECEvent event2(0xdeadbeef, 100);
  CSPECEvent event3(0xdeadbeef, 101);

  ASSERT_FALSE(event1 < event1);
  ASSERT_FALSE(event1 > event1);
  ASSERT_FALSE(event1 < event2);
  ASSERT_FALSE(event1 > event2);

  ASSERT_TRUE(event1 < event3);
  ASSERT_FALSE(event1 > event3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
