/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/CdtFile.h>
#include <test/TestBase.h>

using namespace Jalousie;

class CdtFileTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CdtFileTest, DoSomething) {
  CdtFile file(TEST_DATA_PATH "noise.bin");
  EXPECT_EQ(file.stats.events_found, 249773);
  EXPECT_EQ(file.stats.pulses_found, 5812);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
