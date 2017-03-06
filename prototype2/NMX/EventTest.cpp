/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/EventNMX.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class PlaneTest : public TestBase {
protected:
  PlaneNMX *plane;
  virtual void SetUp() { plane = new PlaneNMX(); }
  virtual void TearDown() { delete plane; }
};

TEST_F(PlaneTest, Insert) {
  Eventlet e1;
  plane->insert_eventlet(e1);
  ASSERT_EQ(plane->entries.size(), 0);
  e1.adc = 1;
  plane->insert_eventlet(e1);
  ASSERT_EQ(plane->entries.size(), 1);
  plane->insert_eventlet(e1);
  ASSERT_EQ(plane->entries.size(), 2);
}

class EventTest : public TestBase {
protected:
  EventNMX *event;
  virtual void SetUp() { event = new EventNMX(); }
  virtual void TearDown() { delete event; }
};

TEST_F(EventTest, Insert) {
  Eventlet e1;
  e1.adc = 1;
  event->insert_eventlet(e1);
  ASSERT_EQ(event->x.entries.size(), 1);
  e1.plane_id = 1;
  event->insert_eventlet(e1);
  ASSERT_EQ(event->y.entries.size(), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
