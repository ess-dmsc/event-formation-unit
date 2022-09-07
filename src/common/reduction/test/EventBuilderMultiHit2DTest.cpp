/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/EventBuilderMultiHit2D.h>
#include <common/testutils/TestBase.h>

class EventBuilderMultiHit2DTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(EventBuilderMultiHit2DTest, insertHit){
  EventBuilderMultiHit2D builder;
  Hit e;
  builder.insert(e);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
