/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Clusterer.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class ClustererTest : public TestBase {
protected:
  Clusterer *clusterer;
  virtual void SetUp() { clusterer = new Clusterer(30); }
  virtual void TearDown() { delete clusterer; }
};

TEST_F(ClustererTest, Insert) {
  ASSERT_FALSE(clusterer->event_ready());
  Eventlet e1;
  clusterer->insert(e1);
  ASSERT_FALSE(clusterer->event_ready());
  e1.time = 12;
  clusterer->insert(e1);
  ASSERT_FALSE(clusterer->event_ready());
  e1.time = 32;
  clusterer->insert(e1);
  ASSERT_TRUE(clusterer->event_ready());
}

TEST_F(ClustererTest, GetBadEvent) {
  Eventlet e1, e2;
  e1.adc = e2.adc = 1;
  e2.time = 32;
  clusterer->insert(e1);
  clusterer->insert(e2);
  ASSERT_TRUE(clusterer->event_ready());
  auto event = clusterer->get_event();
  ASSERT_FALSE(event.good());
}

TEST_F(ClustererTest, GetEvent) {
  auto event1 = clusterer->get_event();
  event1.analyze(true, 3, 7);
  ASSERT_FALSE(event1.good());

  Eventlet e1, e2;
  e1.adc = e2.adc = 1;
  e2.plane_id = 1;
  clusterer->insert(e1);
  clusterer->insert(e2);
  e1.time = e2.time = 32;
  clusterer->insert(e1);
  clusterer->insert(e2);
  auto event2 = clusterer->get_event();
  event2.analyze(true, 3, 7);
  ASSERT_TRUE(event2.good());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
