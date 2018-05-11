/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <gdgem/nmx/Event.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventTest : public TestBase {
protected:
  Event event;
  virtual void SetUp() {  }
  virtual void TearDown() {  }
};

TEST_F(EventTest, Insert) {
  Eventlet e1;
  e1.adc = 1;
  event.insert_eventlet(e1);
  ASSERT_EQ(event.x.entries.size(), 1);
  e1.plane_id = 1;
  event.insert_eventlet(e1);
  ASSERT_EQ(event.y.entries.size(), 1);
}

TEST_F(EventTest, AnalyzeBadY) {
  Eventlet e1;
  e1.adc = 1;
  event.insert_eventlet(e1);
  event.analyze(true, 5, 5);
  ASSERT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeBadX) {
  Eventlet e1;
  e1.adc = 1;
  e1.plane_id = 1;
  event.insert_eventlet(e1);
  event.analyze(true, 5, 5);
  ASSERT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeGood) {
  Eventlet e1;
  e1.adc = 1;
  event.insert_eventlet(e1);
  e1.plane_id = 1;
  event.insert_eventlet(e1);
  event.analyze(true, 5, 5);
  ASSERT_TRUE(event.valid());
}

TEST_F(EventTest, InsertInvalid) {
  Eventlet e1;
  e1.adc = 1;
  e1.plane_id = 0;
  event.insert_eventlet(e1);
  e1.plane_id = 1;
  event.insert_eventlet(e1);
  e1.plane_id = 2;
  event.insert_eventlet(e1);
  ASSERT_EQ(2, event.x.entries.size() + event.y.entries.size());
}

TEST_F(EventTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  Eventlet e1;
  e1.adc = 1;
  event.insert_eventlet(e1);
  e1.plane_id = 1;
  event.insert_eventlet(e1);
  event.analyze(true, 5, 5);
  auto debugstr = event.debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
