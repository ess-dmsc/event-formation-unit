/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/RingBuffer.h>
#include <test/TestBase.h>

class RingBufferTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(RingBufferTest, Constructor) {
  RingBuffer<9000> buf(100);
  ASSERT_EQ(buf.getMaxElements(), 100);
  ASSERT_EQ(buf.getMaxBufSize(), 9000);
}

TEST_F(RingBufferTest, Datalength) {
  RingBuffer<9000> buf(100);
  unsigned int index = buf.getDataIndex();
  buf.setDataLength(index, 9000);
  ASSERT_EQ(9000, buf.getDataLength(index));
  buf.setDataLength(index, 900);
  ASSERT_EQ(900, buf.getDataLength(index));
}

TEST_F(RingBufferTest, CircularWrap) {
  int N = 997;
  const int size = 9000;
  RingBuffer<size> buf(N);

  ASSERT_EQ(buf.getMaxElements(), N);
  ASSERT_EQ(buf.getMaxBufSize(), size);

  unsigned int index = buf.getDataIndex();
  ASSERT_EQ(index, 0);

  char *first = buf.getDataBuffer(index); // Save for last

  for (int i = 0; i < N; i++) {
    unsigned int InnerIndex= buf.getDataIndex();
    ASSERT_EQ(i, InnerIndex);

    int *ip = (int *)buf.getDataBuffer(InnerIndex);
    ASSERT_NE(ip, nullptr);
    ASSERT_EQ(i, buf.getDataIndex());
    *ip = N + i;
    buf.getNextBuffer();
  }
  ASSERT_EQ(0, buf.getDataIndex());

  for (int i = 0; i < N; i++) {
    unsigned int InnerIndex = buf.getDataIndex();
    int *ip = (int *)buf.getDataBuffer(InnerIndex);
    ASSERT_EQ(*ip, N + i);
    buf.getNextBuffer();
  }
  index = buf.getDataIndex();
  ASSERT_EQ(first, buf.getDataBuffer(index));
}

TEST_F(RingBufferTest, OverWriteLocal) {
  RingBuffer<9000> buf(2);
  unsigned int index = buf.getDataIndex();
  char *buffer = buf.getDataBuffer(index);
  std::fill_n(buffer, 9001, 0);

  MESSAGE() << "Next buffer should be ok\n";
  index = buf.getNextBuffer();
  ASSERT_TRUE(buf.verifyBufferCookies(index));

  MESSAGE() << "Next2 buffer should be corrupt\n";
  index = buf.getNextBuffer();
  ASSERT_FALSE(buf.verifyBufferCookies(index));
}

TEST_F(RingBufferTest, OverWriteNext) {
  RingBuffer<9000> buf(2);
  unsigned int index = buf.getDataIndex();
  char *buffer = buf.getDataBuffer(index);
  std::fill_n(buffer, 9005, 0);
  MESSAGE() << "Next buffer should be corrupt\n";
  index = buf.getNextBuffer();
  ASSERT_FALSE(buf.verifyBufferCookies(index));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
