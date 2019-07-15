/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/ReducedEvent.h>

#include <test/TestBase.h>

class ReducedEventTest : public TestBase {
protected:
  ReducedEvent event;
  void SetUp() override { }
  void TearDown() override { }
};

// \todo tests needed

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
