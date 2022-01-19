/** Copyright (C) 2018 European Spallation Source */

#include <common/testutils/TestBase.h>
#include <common/BitMath.h>

struct pair16 {
  uint16_t input;
  uint16_t output;
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

TEST_F(BitMathTest, GrayCode) {
  ASSERT_FALSE(true);
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
