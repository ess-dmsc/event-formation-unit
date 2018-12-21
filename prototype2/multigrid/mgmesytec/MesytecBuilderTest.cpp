/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MesytecBuilder.h>
#include <multigrid/mgmesytec/TestData.h>
#include <test/TestBase.h>

using namespace Multigrid;

class MesytecBuilderTest : public TestBase {
protected:
  MesytecBuilder builder;
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

// \todo use reference config and data

TEST_F(MesytecBuilderTest, ErrNoTimeStamp) {
  builder.parse(err_no_timestamp);

  // \todo ensure strong guarantee?
}

//TEST_F(MesytecBuilderTest, ParseRecordedWSData) {
//  builder.parse(ws1);
//
//  EXPECT_EQ(builder.ConvertedData.size(), 128);
//}
//
//TEST_F(MesytecBuilderTest, ParseRecordedWSDataII) {
//  builder.parse(ws2);
//
//  EXPECT_EQ(builder.ConvertedData.size(), 128);
//}
//
//TEST_F(MesytecBuilderTest, ParseRecordedWSDataIII) {
//  builder.parse(ws3);
//
//  EXPECT_EQ(builder.ConvertedData.size(), 128);
//}
//
//TEST_F(MesytecBuilderTest, ParseRecordedWSDataMultipleTriggers) {
//  builder.parse(ws4);
//
//  EXPECT_EQ(builder.ConvertedData.size(), 128);
//}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
