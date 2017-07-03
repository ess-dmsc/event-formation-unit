/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <gdgem/nmx/EventNMX.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

#include <iostream>

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
  e1.strip = 1;
  plane->insert_eventlet(e1);
  ASSERT_EQ(plane->entries.size(), 2);
}

TEST_F(PlaneTest, AnalyzeInvalid) {
  ASSERT_TRUE(std::isnan(plane->center));
  plane->analyze(false, 2, 2);
  ASSERT_TRUE(std::isnan(plane->center));
}

TEST_F(PlaneTest, AnalyzeAverage) {
  Eventlet e1;
  e1.strip = 0;
  e1.adc = 2;
  plane->insert_eventlet(e1);
  plane->analyze(false, 1, 1);
  ASSERT_EQ(plane->center, 0);
  e1.strip = 1;
  e1.adc = 4;
  plane->insert_eventlet(e1);
  e1.strip = 2;
  e1.adc = 4;
  plane->insert_eventlet(e1);
  plane->analyze(false, 1, 1);
  ASSERT_EQ(plane->entries.size(), 3);
  ASSERT_EQ(plane->center, 1);
  plane->analyze(true, 1, 1);
  ASSERT_EQ(plane->center, 1.2);
}

TEST_F(PlaneTest, AnalyzeUncert) {
  Eventlet e1;
  e1.adc = 1;

  e1.time = e1.strip = 0;
  plane->insert_eventlet(e1);
  e1.time = e1.strip = 1;
  plane->insert_eventlet(e1);
  e1.time = e1.strip = 2;
  plane->insert_eventlet(e1);

  plane->analyze(true, 1, 1);
  ASSERT_EQ(plane->center, 2);
  ASSERT_EQ(plane->uncert_lower, 1);
  ASSERT_EQ(plane->uncert_upper, 1);

  plane->analyze(true, 2, 2);
  ASSERT_EQ(plane->center, 2);
  ASSERT_EQ(plane->uncert_lower, 1);
  ASSERT_EQ(plane->uncert_upper, 2);

  e1.strip = 1;
  plane->insert_eventlet(e1);
  plane->analyze(true, 2, 2);
  ASSERT_EQ(plane->center, 1.5);
  ASSERT_EQ(plane->uncert_lower, 2);
  ASSERT_EQ(plane->uncert_upper, 2);

  plane->analyze(true, 5, 5);
  ASSERT_EQ(plane->center, 1.5);
  ASSERT_EQ(plane->uncert_lower, 2);
  ASSERT_EQ(plane->uncert_upper, 3);
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

TEST_F(EventTest, AnalyzeBadY) {
  Eventlet e1;
  e1.adc = 1;
  event->insert_eventlet(e1);
  event->analyze(true, 5, 5);
  ASSERT_FALSE(event->good());
}

TEST_F(EventTest, AnalyzeBadX) {
  Eventlet e1;
  e1.adc = 1;
  e1.plane_id = 1;
  event->insert_eventlet(e1);
  event->analyze(true, 5, 5);
  ASSERT_FALSE(event->good());
}

TEST_F(EventTest, AnalyzeGood) {
  Eventlet e1;
  e1.adc = 1;
  event->insert_eventlet(e1);
  e1.plane_id = 1;
  event->insert_eventlet(e1);
  event->analyze(true, 5, 5);
  ASSERT_TRUE(event->good());
}

TEST_F(EventTest, InsertInvalid) {
  Eventlet e1;
  e1.adc = 1;
  e1.plane_id = 0;
  event->insert_eventlet(e1);
  e1.plane_id = 1;
  event->insert_eventlet(e1);
  e1.plane_id = 2;
  event->insert_eventlet(e1);
  ASSERT_EQ(2, event->x.entries.size() + event->y.entries.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
