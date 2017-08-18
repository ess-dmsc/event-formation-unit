/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmxgen/EventletBuilderH5.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventletBuilderH5Test : public TestBase {
protected:
  EventletBuilderH5 *builder;
  virtual void SetUp() { builder = new EventletBuilderH5(); }
  virtual void TearDown() { delete builder; }
};

TEST_F(EventletBuilderH5Test, Process) {
  Clusterer clusterer(30);
  char data[9000];

  auto num1 = builder->process_readout(data, 16, clusterer);
  ASSERT_EQ(num1, 1);

  auto num2 = builder->process_readout(data, 64, clusterer);
  ASSERT_EQ(num2, 4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
