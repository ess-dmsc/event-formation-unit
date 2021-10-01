/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Buffer.h>
#include <memory>
#include <common/testutils/TestBase.h>

class BufferTest : public TestBase {
};

TEST_F(BufferTest, Default) {
  Buffer<int64_t> b;
  EXPECT_FALSE(static_cast<bool>(b));
  EXPECT_EQ(b.size, 0);
  EXPECT_EQ(b.bytes(), 0);
}

TEST_F(BufferTest, FromVector) {
  std::vector<uint32_t> v {1,2,3,4,5};
  auto b = Buffer<uint32_t>(v);
  EXPECT_TRUE(static_cast<bool>(b));
  EXPECT_EQ(b.size, 5);
  EXPECT_EQ(b.bytes(), 20);
}

TEST_F(BufferTest, Bytes) {
  std::vector<uint8_t> u8 {1,2,3,4,5};
  auto b8 = Buffer<uint8_t>(u8);
  EXPECT_EQ(b8.bytes(), 5);

  std::vector<uint16_t> u16 {1,2,3,4,5};
  auto b16 = Buffer<uint16_t>(u16);
  EXPECT_EQ(b16.bytes(), 10);

  std::vector<uint32_t> u32 {1,2,3,4,5};
  auto b32 = Buffer<uint32_t>(u32);
  EXPECT_EQ(b32.bytes(), 20);

  std::vector<uint64_t> u64 {1,2,3,4,5};
  auto b64 = Buffer<uint64_t>(u64);
  EXPECT_EQ(b64.bytes(), 40);
}

TEST_F(BufferTest, ElementAccess) {
  std::vector<uint32_t> v {1,2,3,4,5};
  auto b = Buffer<uint32_t>(v);
  for (size_t i=0; i < 5; ++i) {
    EXPECT_EQ(b[i], i+1);
    EXPECT_EQ(b.at(i), i+1);
  }
}

TEST_F(BufferTest, Increment) {
  std::vector<uint32_t> v {1,2,3,4,5};
  auto b = Buffer<uint32_t>(v);
  EXPECT_EQ(b.size, 5);
  b++;
  EXPECT_EQ(b.size, 4);
  ++b;
  EXPECT_EQ(b.size, 3);
  b+=2;
  EXPECT_EQ(b.size, 1);
}

TEST_F(BufferTest, Dereference) {
  std::vector<uint32_t> v {1,2,3,4,5};
  auto b = Buffer<uint32_t>(v);
  uint32_t i=1;
  for (; b.size; b++) {
    EXPECT_EQ(*b, i++);
  }
}

TEST_F(BufferTest, Conversion) {
  std::vector<uint8_t> v {128,0,2,2,0,1};
  auto b8 = Buffer<uint8_t>(v);
  Buffer<uint16_t> b16(b8);
  EXPECT_EQ(b16.size, 3);
  EXPECT_EQ(b16.bytes(), 6);
  EXPECT_EQ(b16[0], 128);
  EXPECT_EQ(b16[1], 514);
  EXPECT_EQ(b16[2], 256);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
