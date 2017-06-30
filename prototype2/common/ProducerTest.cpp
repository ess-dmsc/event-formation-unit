/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Producer.h>
#include <test/TestBase.h>
#include <cstring>
#include <librdkafka/rdkafkacpp.h>


class ProducerTest : public TestBase {
    virtual void SetUp() {}
    virtual void TearDown() {}

protected:

};

TEST_F(ProducerTest, ConstructorInvalidStrings) {
  Producer prod{"", "notopic"};
  MESSAGE() << "hello\n";
  auto ret = prod.produce(0, -1);
  MESSAGE() << "hello2\n";
  ASSERT_EQ(ret, RdKafka::ERR_NO_ERROR);
  //Producer prod2{"nobroker", ""};
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
