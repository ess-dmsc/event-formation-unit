/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multiblade/mbcaen/MultiBladeGeometry.h>
#include <test/TestBase.h>

class MultiBladeGeometryTest : public TestBase {};

const int Ns = 32;
const int Nw = 32;
const int Nc = 6;

/** Test cases below */
TEST_F(MultiBladeGeometryTest, simple) {
  MultiBladeGeometry mbg;

  // Top cassette (idx 0)
  ASSERT_EQ( 1, mbg.pixelid(137, 32, 1)); // Top left
  ASSERT_EQ(32, mbg.pixelid(137,  1, 1)); // Top right
  ASSERT_EQ(33, mbg.pixelid(137, 32, 2)); // Second row left
  ASSERT_EQ(64, mbg.pixelid(137,  1, 2)); // Second row right
  ASSERT_EQ(Ns * Nw - 31, mbg.pixelid(137,  32, 32)); // Last row left
  ASSERT_EQ(Ns * Nw, mbg.pixelid(137,  1, 32)); // Last row right

  // Second cassette (idx 1)
  ASSERT_EQ(Ns * Nw +  1, mbg.pixelid(143, 32, 1)); // Top left
  ASSERT_EQ(Ns * Nw + Ns, mbg.pixelid(143,  1, 1)); // Top right

  // Third cassette (idx 2)
  ASSERT_EQ(2  *Ns * Nw +  1, mbg.pixelid(142, 32, 1)); // Top left
  ASSERT_EQ(2 * Ns * Nw + Ns, mbg.pixelid(142,  1, 1)); // Top right

  // Fourth cassette (idx 3)
  ASSERT_EQ(3 * Ns * Nw +  1, mbg.pixelid(31, 32, 1)); // Top left
  ASSERT_EQ(3 * Ns * Nw + Ns, mbg.pixelid(31,  1, 1)); // Top right

  // Fifth cassette (idx 4)
  ASSERT_EQ(4 * Ns * Nw +  1, mbg.pixelid(34, 32, 1)); // Top left
  ASSERT_EQ(4 * Ns * Nw + Ns, mbg.pixelid(34,  1, 1)); // Top right

  // Sixth cassette (idx 5)
  ASSERT_EQ(5 * Ns * Nw +  1, mbg.pixelid(33, 32, 1)); // Top left
  ASSERT_EQ(5 * Ns * Nw + Ns, mbg.pixelid(33,  1, 1)); // Top right
  ASSERT_EQ(Ns * Nw * Nc, mbg.pixelid(33,  1, 32)); // Last row right
}

TEST_F(MultiBladeGeometryTest, InvalidDigitizerID) {
  MultiBladeGeometry mbg;

  ASSERT_EQ( 0, mbg.pixelid(0, 1, 1));
  ASSERT_EQ( 0, mbg.pixelid(33, 0, 1));
  ASSERT_EQ( 0, mbg.pixelid(33, Ns + 1, 1));
  ASSERT_EQ( 0, mbg.pixelid(33, 1, 0));
  ASSERT_EQ( 0, mbg.pixelid(33, 1, Nw + 1));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
