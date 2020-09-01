/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <readout/ESSTime.h>
#include <test/TestBase.h>

class ESSTimeTest : public TestBase {
protected:
  ESSTime Time;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ESSTimeTest, Constructor) {
  ASSERT_EQ(Time.getTOF(0, 0), 0);
}

TEST_F(ESSTimeTest, SetRef) {
  Time.setReference(100, 0);
  ASSERT_EQ(Time.getTOF(100, 0),  0);
  ASSERT_EQ(Time.getTOF(100, 1), 11);
  ASSERT_EQ(Time.getTOF(100, 2), 22);
  ASSERT_EQ(Time.getTOF(100, 3), 34);
  ASSERT_EQ(Time.getTOF(200, 0), 100*1000000000LU);

}

TEST_F(ESSTimeTest, Bounds) {
  Time.setReference(0, 0);
  ASSERT_EQ(Time.getTOF(0, 88052499), 999999988);
  Time.setReference(0, 88052499);
  ASSERT_EQ(Time.getTOF(1, 0), 12); // why not 11?
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
