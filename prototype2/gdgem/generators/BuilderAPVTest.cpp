/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderAPV.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class BuilderAPVTest : public TestBase {
protected:
  BuilderAPV *builder;
  virtual void SetUp() { builder = new BuilderAPV("", false, false); }
  virtual void TearDown() { delete builder; }
};

TEST_F(BuilderAPVTest, Process) {
  NMXHists hists;
  Clusterer clusterer(30);
  // char data[9000];

  // auto num1 = builder->process_buffer(data, 16, clusterer, hists);
  // ASSERT_EQ(num1.valid_eventlets, 1);

  // auto num2 = builder->process_buffer(data, 64, clusterer, hists);
  // ASSERT_EQ(num2.valid_eventlets, 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
