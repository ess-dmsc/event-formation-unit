/** Copyright (C) 2018 European Spallation Source */

#include <prototype2/test/TestBase.h>
#include <common/BitMath.h>

struct pair16 {
  uint16_t input;
  uint16_t output;
};

struct pair32 {
  uint32_t input;
  uint32_t output;
};

std::vector<struct pair16> results16 = {
  {0x0201, 0x8040},
  {0xaaaa, 0x5555},
  {0x5555, 0xaaaa},
  {0xaa55, 0xaa55},
  {0xaaa5, 0xa555},
  {0xaa5a, 0x5a55},
  {0xa5aa, 0x55a5},
  {0x5aaa, 0x555a},
  {0xa5a5, 0xa5a5},
  {0x5a5a, 0x5a5a}
};

std::vector<struct pair32> results32 = {
  {0x08040201, 0x80402010},
  {0xaaaaaaaa, 0x55555555},
  {0x55555555, 0xaaaaaaaa},
  {0xaaaa5555, 0xaaaa5555},
  {0xaaa5555a, 0x5aaaa555},
  {0xaa5555aa, 0x55aaaa55},
  {0xa5555aaa, 0x555aaaa5},
  {0x5555aaaa, 0x5555aaaa},
  {0xa5a5a5a5, 0xa5a5a5a5},
  {0x5a5a5a5a, 0x5a5a5a5a}
};

// Is this a correct table?
std::vector<struct pair16> graycode = {
  { 0,  0}, { 1,  1}, { 3,  2}, { 2,  3},
  { 6,  4}, { 7,  5}, { 5,  6}, { 4,  7},
  {12,  8}, {13,  9}, {15, 10}, {14, 11},
  {10, 12}, {11, 13}, { 9, 14}, { 8, 15}
};

class BitMathTest : public ::testing::Test {
protected:
};

TEST_F(BitMathTest, BitSawp32) {
  for (auto data : results32) {
    ASSERT_EQ(BitMath::reversebits32(data.input), data.output);
  }
}

TEST_F(BitMathTest, BitSawp16) {
  for (auto data : results16) {
    ASSERT_EQ(BitMath::reversebits16(data.input), data.output);
  }
}

TEST_F(BitMathTest, GrayCode) {
  for (auto data : graycode) {
    auto gray_code = data.input;
    auto binary = BitMath::gray2bin32(gray_code);
    printf("i: %d, gray2bin32(i): %d\n", gray_code, binary);
    ASSERT_EQ(binary, data.output);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
