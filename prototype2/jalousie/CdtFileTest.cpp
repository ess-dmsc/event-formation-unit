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

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
