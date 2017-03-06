/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/RingBuffer.h>
#include <test/TestBase.h>

using namespace std;

class RingBufferTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/** Test cases below */
TEST_F(RingBufferTest, Constructor) {
  RingBuffer<9000> buf(100);
  ASSERT_EQ(buf.getmaxelems(), 100);
  ASSERT_EQ(buf.getmaxbufsize(), 9000);
}

TEST_F(RingBufferTest, Datalength) {
  RingBuffer<9000> buf(100);
  unsigned int index = buf.getindex();
  buf.setdatalength(index, 9000);
  ASSERT_EQ(9000, buf.getdatalength(index));
  buf.setdatalength(index, 900);
  ASSERT_EQ(900, buf.getdatalength(index));
}

TEST_F(RingBufferTest, CircularWrap) {
  int N = 997;
  int size = 9000;
  RingBuffer<9000> buf(N);

  unsigned int index = buf.getindex();
  char *first = buf.getdatabuffer(index);
  ASSERT_EQ(buf.getmaxelems(), N);
  ASSERT_EQ(buf.getmaxbufsize(), size);

  for (int i = 0; i < N; i++) {
    unsigned int index = buf.getindex();
    int *ip = (int *)buf.getdatabuffer(index);
    ASSERT_NE(ip, nullptr);
    ASSERT_EQ(i, buf.getindex());
    *ip = N + i;
    buf.nextbuffer();
  }
  ASSERT_EQ(0, buf.getindex());

  for (int i = 0; i < N; i++) {
    unsigned int index = buf.getindex();
    int *ip = (int *)buf.getdatabuffer(index);
    ASSERT_EQ(*ip, N + i);
    buf.nextbuffer();
  }
  index = buf.getindex();
  ASSERT_EQ(first, buf.getdatabuffer(index));
}

TEST_F(RingBufferTest, OverWriteLocal) {
  RingBuffer<9000> buf(2);
  unsigned int index = buf.getindex();
  char *buffer = buf.getdatabuffer(index);
  std::fill_n(buffer, 9001, 0);
  MESSAGE() << "Next buffer should be ok\n";
  buf.nextbuffer();
  MESSAGE() << "Next2 buffer should be corrupt\n";
  ASSERT_DEATH(buf.nextbuffer(), "COOKIE2");
}

TEST_F(RingBufferTest, OverWriteNext) {
  RingBuffer<9000> buf(2);
  unsigned int index = buf.getindex();
  char *buffer = buf.getdatabuffer(index);
  std::fill_n(buffer, 9005, 0);
  MESSAGE() << "Next buffer should be corrupt\n";
  ASSERT_DEATH(buf.nextbuffer(), "COOKIE1");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
