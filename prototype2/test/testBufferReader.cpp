/** Copyright (C) 2016 European Spallation Source */

#include "TestBase.h"
#include <RingBuffer.h>

using namespace std;

class RingBufferTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/** Test cases below */
TEST_F(RingBufferTest, Constructor) {
  RingBuffer buf(1000, 100);
  ASSERT_EQ(buf.getelems(), 100);
  ASSERT_EQ(buf.getsize(), 1000);
}

TEST_F(RingBufferTest, CircularWrap) {
  int N = 997;
  int size = 9000;
  RingBuffer buf(size, N);

  char * first = buf.getbuffer();
  ASSERT_EQ(buf.getelems(), N);
  ASSERT_EQ(buf.getsize(), size);

  for (int i = 0; i < N; i++) {
    int  * ip = (int *)buf.getbuffer();
    ASSERT_NE(ip, nullptr);
    ASSERT_EQ(i, buf.getentry());
    *ip = N + i;
    buf.nextbuffer();
  }
  ASSERT_EQ(0, buf.getentry());

  for (int i = 0; i < N; i++) {
    int  * ip = (int *)buf.getbuffer();
    ASSERT_EQ(*ip, N + i);
    buf.nextbuffer();
  }
  ASSERT_EQ(first, buf.getbuffer());
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
