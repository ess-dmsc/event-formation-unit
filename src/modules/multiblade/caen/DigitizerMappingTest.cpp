/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multiblade/caen/DigitizerMapping.h>
#include <common/testutils/TestBase.h>

class DigitizerMappingTest : public TestBase {};

std::vector<struct Multiblade::DigitizerMapping::Digitiser> digits {
  {0, 137} , {1, 143}, {2, 142}, {3, 31}, {4, 33}, {5, 34}
};

/** Test cases below */
TEST_F(DigitizerMappingTest, CassetteValid) {
  Multiblade::DigitizerMapping mbg(digits);

  ASSERT_EQ(0, mbg.cassette(137));
  ASSERT_EQ(1, mbg.cassette(143));
  ASSERT_EQ(2, mbg.cassette(142));
  ASSERT_EQ(3, mbg.cassette(31));
  ASSERT_EQ(4, mbg.cassette(33));
  ASSERT_EQ(5, mbg.cassette(34));
}

TEST_F(DigitizerMappingTest, CassetteInValid) {
  Multiblade::DigitizerMapping mbg(digits);

  ASSERT_EQ(-1, mbg.cassette(-1));
  ASSERT_EQ(-1, mbg.cassette(0));
  ASSERT_EQ(-1, mbg.cassette(30));
  ASSERT_EQ(-1, mbg.cassette(32));
  ASSERT_EQ(-1, mbg.cassette(35));
  ASSERT_EQ(-1, mbg.cassette(136));
  ASSERT_EQ(-1, mbg.cassette(138));
  ASSERT_EQ(-1, mbg.cassette(141));
  ASSERT_EQ(-1, mbg.cassette(144));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
