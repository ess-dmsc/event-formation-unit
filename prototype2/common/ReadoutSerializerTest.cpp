/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/ReadoutSerializer.h>
#include <common/Producer.h>
#include <test/TestBase.h>

class ReadoutSerializerTest : public TestBase {
  virtual void SetUp() {  }

  virtual void TearDown() {  }

protected:

};

TEST_F(ReadoutSerializerTest, Constructor) {
  Producer readoutproducer{"nobroker", "notopic"};
  ReadoutSerializer(10000, readoutproducer);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
