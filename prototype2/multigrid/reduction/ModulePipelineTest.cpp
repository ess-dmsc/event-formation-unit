/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/ModulePipeline.h>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <common/DynamicHist.h>
#include <test/TestBase.h>

using namespace Multigrid;

class ModulePipelineTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ModulePipelineTest, t00004) {

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
