/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Eventlet.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventletTest : public TestBase {
protected:
  Eventlet *eventlet;
  virtual void SetUp() { eventlet = new Eventlet(); }
  virtual void TearDown() { delete eventlet; }
};

TEST_F(EventletTest, Debug) {
  ASSERT_FALSE(eventlet->debug().empty());
  eventlet->over_threshold = true;
  ASSERT_FALSE(eventlet->debug().empty());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
