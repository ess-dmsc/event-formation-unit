// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <gtest/gtest.h>
#include "../EventBuffer.h"

class EventBufferTest : public ::testing::Test {
public:
  void SetUp() override {
    TestEvent = std::unique_ptr<EventData>(new EventData{Timestamp, EventId, 1, 2, 3, ThreasholdTime, PeakTime});
  }
  const std::uint64_t Timestamp{100};
  const std::uint32_t EventId{42};
  const std::uint64_t ThreasholdTime{101};
  const std::uint64_t PeakTime{102};
  std::unique_ptr<EventData> TestEvent;
};

TEST_F(EventBufferTest, AddOneEventSuccess) {
  EventBuffer UnderTest(1);
  EXPECT_TRUE(UnderTest.addEvent(TestEvent));
  auto BufferedEvents = UnderTest.getAllEvents();
  EXPECT_EQ(BufferedEvents.size(), 1u);
  auto CurrentEvent = BufferedEvents[0];
  EXPECT_EQ(CurrentEvent, *TestEvent);
}

TEST_F(EventBufferTest, AddTwoEventsFail) {
  EventBuffer UnderTest(1);
  EXPECT_TRUE(UnderTest.addEvent(TestEvent));
  EXPECT_FALSE(UnderTest.addEvent(TestEvent));
}

TEST_F(EventBufferTest, DoNotCullEventsOnEmptyBuffer) {
  EventBuffer UnderTest(1);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, DoNotCullEventsOnBufferNotFull) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull1) {
  EventBuffer UnderTest(1);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull2) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull3) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ClearOnEmpty) {
  EventBuffer UnderTest(2);
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

TEST_F(EventBufferTest, ClearOnOneEvent) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  EXPECT_EQ(UnderTest.getEvents().size(), 1u);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

  TEST_F(EventBufferTest, ClearOnTwoEvents) {
    EventBuffer UnderTest(2);
    UnderTest.addEvent(TestEvent);
    UnderTest.addEvent(TestEvent);
    EXPECT_EQ(UnderTest.getEvents().size(), 2u);
    UnderTest.clearEvents();
    EXPECT_EQ(UnderTest.getEvents().size(), 0u);
  }

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(0, 2000);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(90, 2000);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(0, 999);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(99, 1000);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldNotCullOnOutsideTimeRange) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 50);
  EXPECT_FALSE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 50);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange4) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 100;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 50);
  EXPECT_TRUE(UnderTest.shouldCullEvents());
}


TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange1) {
  EventBuffer UnderTest(3);
  std::uint64_t TimeOne{100};
  std::uint64_t TimeTwo{1100};
  TestEvent->Timestamp = TimeOne;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeTwo;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(0, 999);
  ASSERT_EQ(UnderTest.getEvents().size(), 1u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, TimeOne);
  UnderTest.clearEvents();
  ASSERT_EQ(UnderTest.getEvents().size(), 1u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, TimeTwo);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange2) {
  EventBuffer UnderTest(3);
  std::uint64_t TimeOne{100};
  std::uint64_t TimeTwo{1100};
  TestEvent->Timestamp = TimeOne;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeTwo;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(99, 1000);
  ASSERT_EQ(UnderTest.getEvents().size(), 1u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, TimeOne);
  UnderTest.clearEvents();
  ASSERT_EQ(UnderTest.getEvents().size(), 1u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, TimeTwo);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 50);
  ASSERT_EQ(UnderTest.getEvents().size(), 3u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, 100ul);
  EXPECT_EQ(UnderTest.getEvents()[1].Timestamp, 110ul);
  EXPECT_EQ(UnderTest.getEvents()[2].Timestamp, 120ul);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange4) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 15);
  ASSERT_EQ(UnderTest.getEvents().size(), 2u);
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, 100ul);
  EXPECT_EQ(UnderTest.getEvents()[1].Timestamp, 110ul);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents()[0].Timestamp, 120ul);
  UnderTest.clearEvents();
  EXPECT_EQ(UnderTest.getEvents().size(), 0u);
}

TEST_F(EventBufferTest, GetAllEvents1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.setReferenceTimes(10, 15);
  ASSERT_EQ(UnderTest.getAllEvents().size(), 3u);
  UnderTest.clearAllEvents();
  EXPECT_EQ(UnderTest.getAllEvents().size(), 0u);
}

TEST_F(EventBufferTest, GetAllEvents2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  ASSERT_EQ(UnderTest.getAllEvents().size(), 2u);
  UnderTest.clearAllEvents();
  EXPECT_EQ(UnderTest.getAllEvents().size(), 0u);
}
