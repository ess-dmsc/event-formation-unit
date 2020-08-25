// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "../EventBuffer.h"
#include <gtest/gtest.h>

class EventBufferTest : public ::testing::Test {
public:
  void SetUp() override {
    TestEvent = std::unique_ptr<EventData>(
        new EventData{Timestamp, EventId, 1, 2, 3, ThreasholdTime, PeakTime});
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
  EXPECT_EQ(BufferedEvents.Events.size(), 1u);
  auto CurrentEvent = BufferedEvents.Events[0];
  EXPECT_EQ(CurrentEvent, *TestEvent);
}

TEST_F(EventBufferTest, AddTwoEventsFail) {
  EventBuffer UnderTest(1);
  EXPECT_TRUE(UnderTest.addEvent(TestEvent));
  EXPECT_FALSE(UnderTest.addEvent(TestEvent));
}

TEST_F(EventBufferTest, DoNotCullEventsOnEmptyBuffer) {
  EventBuffer UnderTest(1);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, DoNotCullEventsOnBufferNotFull) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull1) {
  EventBuffer UnderTest(1);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull2) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, DoCullEventsOnBufferFull3) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ClearOnEmpty) {
  EventBuffer UnderTest(2);
  auto EventListSize = UnderTest.getFrameEvents().Events.size();
  EXPECT_EQ(EventListSize, 0u);
  UnderTest.cullEvents(EventListSize);
  EXPECT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, ClearOnOneEvent) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  auto EventListSize = UnderTest.getFrameEvents().Events.size();
  EXPECT_EQ(EventListSize, 0u);
  UnderTest.clearAllEvents();
  EXPECT_EQ(UnderTest.getAllEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, ClearOnTwoEvents) {
  EventBuffer UnderTest(2);
  UnderTest.addEvent(TestEvent);
  UnderTest.addEvent(TestEvent);
  auto EventListSize = UnderTest.getFrameEvents().Events.size();
  EXPECT_EQ(EventListSize, 2u);
  UnderTest.cullEvents(EventListSize);
  EXPECT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setTimespan(2000);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldNotCullOnInsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(90);
  UnderTest.setTimespan(2000);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.setTimespan(999);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 1000;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(99);
  UnderTest.setTimespan(1000);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldNotCullOnOutsideTimeRange) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(10);
  UnderTest.setTimespan(50);
  EXPECT_FALSE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(10);
  UnderTest.setTimespan(50);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, ShouldCullOnOutsideTimeRange4) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 100;
  UnderTest.addEvent(TestEvent);
  UnderTest.setTimespan(50);
  UnderTest.addReferenceTimestamp(10);
  EXPECT_TRUE(UnderTest.getFrameEvents().Events.size() > 0);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange1) {
  EventBuffer UnderTest(3);
  std::uint64_t TimeOne{100};
  std::uint64_t TimeTwo{1100};
  TestEvent->Timestamp = TimeOne;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeTwo;
  UnderTest.addEvent(TestEvent);
  UnderTest.setTimespan(999);
  auto EventList = UnderTest.getFrameEvents().Events;
  ASSERT_EQ(EventList.size(), 1u);
  EXPECT_EQ(EventList[0].Timestamp, TimeOne);
  UnderTest.cullEvents(EventList.size());
  EventList = UnderTest.getAllEvents().Events;
  ASSERT_EQ(EventList.size(), 1u);
  EXPECT_EQ(EventList[0].Timestamp, TimeTwo);
  UnderTest.cullEvents(EventList.size());
  EXPECT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange2) {
  EventBuffer UnderTest(3);
  std::uint64_t TimeOne{100};
  std::uint64_t TimeTwo{1100};
  TestEvent->Timestamp = TimeOne;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeTwo;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(99);
  UnderTest.setTimespan(1000);
  auto EventList = UnderTest.getFrameEvents().Events;
  ASSERT_EQ(EventList.size(), 1u);
  EXPECT_EQ(EventList[0].Timestamp, TimeOne);
  UnderTest.cullEvents(EventList.size());
  EventList = UnderTest.getAllEvents().Events;
  ASSERT_EQ(EventList.size(), 1u);
  EXPECT_EQ(EventList[0].Timestamp, TimeTwo);
  UnderTest.cullEvents(EventList.size());
  EXPECT_EQ(UnderTest.getAllEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange3) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(10);
  UnderTest.setTimespan(50);
  auto EventList = UnderTest.getFrameEvents().Events;
  ASSERT_EQ(EventList.size(), 3u);
  EXPECT_EQ(EventList[0].Timestamp, 100ul);
  EXPECT_EQ(EventList[1].Timestamp, 110ul);
  EXPECT_EQ(EventList[2].Timestamp, 120ul);
  UnderTest.cullEvents(EventList.size());
  EXPECT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange4) {
  EventBuffer UnderTest(4);
  TestEvent->Timestamp = 100;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(10);
  UnderTest.setTimespan(15);
  auto Events = UnderTest.getFrameEvents();
  auto EventList = Events.Events;
  ASSERT_EQ(EventList.size(), 2u);
  EXPECT_EQ(EventList[0].Timestamp, 100ul);
  EXPECT_EQ(EventList[1].Timestamp, 110ul);
  UnderTest.cullEvents(EventList.size());
  EventList = UnderTest.getAllEvents().Events;
  ASSERT_EQ(EventList.size(), 1u);
  EXPECT_EQ(EventList[0].Timestamp, 120ul);
  UnderTest.cullEvents(EventList.size());
  EXPECT_EQ(UnderTest.getAllEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, GetEventsOnOutsideTimeRange5) {
  EventBuffer UnderTest(5);
  std::uint64_t const TimeOne{100};
  std::uint64_t const TimeTwo{110};
  std::uint64_t const TimeThree{120};
  std::uint64_t const TimeFour{123};
  TestEvent->Timestamp = TimeOne;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeTwo;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeThree;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = TimeFour;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(TimeOne - 10);
  UnderTest.addReferenceTimestamp(TimeOne + 7);
  UnderTest.setTimespan(15);
  auto CEventList = UnderTest.getFrameEvents().Events;
  ASSERT_EQ(CEventList.size(), 1u);
  EXPECT_EQ(CEventList[0].Timestamp, TimeOne);
  UnderTest.cullEvents(CEventList.size());
  CEventList = UnderTest.getFrameEvents().Events;
  ASSERT_EQ(CEventList.size(), 2u);
  EXPECT_EQ(CEventList[0].Timestamp, TimeTwo);
  EXPECT_EQ(CEventList[1].Timestamp, TimeThree);
  UnderTest.cullEvents(CEventList.size());
  CEventList = UnderTest.getAllEvents().Events;
  ASSERT_EQ(CEventList.size(), 1u);
  EXPECT_EQ(CEventList[0].Timestamp, TimeFour);
}

TEST_F(EventBufferTest, GetAllEvents1) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(10);
  UnderTest.setTimespan(50);
  ASSERT_EQ(UnderTest.getAllEvents().Events.size(), 3u);
  UnderTest.clearAllEvents();
  EXPECT_EQ(UnderTest.getAllEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, GetAllEvents2) {
  EventBuffer UnderTest(3);
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 10;
  UnderTest.addEvent(TestEvent);
  ASSERT_EQ(UnderTest.getAllEvents().Events.size(), 2u);
  UnderTest.clearAllEvents();
  EXPECT_EQ(UnderTest.getAllEvents().Events.size(), 0u);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents1) {
  EventBuffer UnderTest(2);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS + 100;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 500;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  EXPECT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents2) {
  EventBuffer UnderTest(3);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS + 100;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5000;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 1u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
  UnderTest.cullEvents(Events.Events.size());
  ASSERT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
  Events = UnderTest.getAllEvents();
  ASSERT_EQ(Events.Events.size(), 1u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 100 + 5000);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents3) {
  EventBuffer UnderTest(3);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS + 3000;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5000;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 1u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 3000);
  UnderTest.cullEvents(Events.Events.size());
  ASSERT_EQ(UnderTest.getFrameEvents().Events.size(), 0u);
  Events = UnderTest.getAllEvents();
  ASSERT_EQ(Events.Events.size(), 1u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 3000 + 5000);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents4) {
  EventBuffer UnderTest(2);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS + 5;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
  UnderTest.cullEvents(Events.Events.size());

  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);

  Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents5) {
  EventBuffer UnderTest(2);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.addReferenceTimestamp(ReferenceTS + 5000);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
  UnderTest.cullEvents(Events.Events.size());

  TestEvent->Timestamp += 5100;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);

  Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 5000);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents6) {
  EventBuffer UnderTest(2);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.addReferenceTimestamp(ReferenceTS + 5000000);
  UnderTest.addReferenceTimestamp(ReferenceTS + 5000);
  UnderTest.addReferenceTimestamp(ReferenceTS + 7000);
  UnderTest.setTimespan(2000);
  auto Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);
  UnderTest.cullEvents(Events.Events.size());

  TestEvent->Timestamp += 5100;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp += 5;
  UnderTest.addEvent(TestEvent);

  Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 5000);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents7) {
  EventBuffer UnderTest(100);
  std::uint64_t ReferenceTS = 1000;
  TestEvent->Timestamp = ReferenceTS + 5;
  TestEvent->Amplitude = 1;
  UnderTest.addEvent(TestEvent);
  TestEvent->Timestamp = ReferenceTS + 10;
  TestEvent->Amplitude = 2;
  UnderTest.addEvent(TestEvent);
  UnderTest.addReferenceTimestamp(ReferenceTS);
  UnderTest.addReferenceTimestamp(ReferenceTS + 20);
  UnderTest.addReferenceTimestamp(ReferenceTS + 15);
  UnderTest.addReferenceTimestamp(ReferenceTS + 15);
  UnderTest.addReferenceTimestamp(ReferenceTS + 20);

  TestEvent->Timestamp = ReferenceTS + 25;
  TestEvent->Amplitude = 3;
  UnderTest.addEvent(TestEvent);

  auto Events = UnderTest.getFrameEvents();
  UnderTest.cullEvents(Events.Events.size());
  ASSERT_EQ(Events.Events.size(), 2u);
  EXPECT_EQ(Events.RefTime, ReferenceTS);

  UnderTest.addReferenceTimestamp(ReferenceTS + 15);
  UnderTest.addReferenceTimestamp(ReferenceTS + 15);

  UnderTest.addReferenceTimestamp(ReferenceTS + 40);
  TestEvent->Timestamp = ReferenceTS + 50;
  TestEvent->Amplitude = 4;
  UnderTest.addEvent(TestEvent);

  Events = UnderTest.getFrameEvents();
  ASSERT_EQ(Events.Events.size(), 1u);
  EXPECT_EQ(Events.RefTime, ReferenceTS + 20);
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents8) {
  ssize_t BufferSize{10};
  EventBuffer UnderTest(BufferSize);
  std::uint64_t const RefTimeInterval{5};
  std::uint64_t CurrentRefTime{1};
  std::uint64_t CurrentTime{1};
  std::uint64_t const EventInterval{1};
  for (int i = 0; i < 100; i++) {
    CurrentTime += EventInterval;
    if (CurrentTime > CurrentRefTime + RefTimeInterval) {
      CurrentRefTime += RefTimeInterval;
    }
    TestEvent->Timestamp = CurrentTime;
    UnderTest.addEvent(TestEvent);
    UnderTest.addReferenceTimestamp(CurrentRefTime);
    auto Events = UnderTest.getFrameEvents();
    if (not Events.Events.empty()) {
      ASSERT_LE(Events.RefTime, Events.Events[0].Timestamp);
      ASSERT_NE(BufferSize, Events.Events.size());
      ASSERT_LE((Events.Events.end() - 1)->Timestamp,
                Events.RefTime + RefTimeInterval);
      UnderTest.cullEvents(Events.Events.size());
    }
  }
}

TEST_F(EventBufferTest, CheckReferenceTimeOnGetEvents9) {
  ssize_t BufferSize{20};
  EventBuffer UnderTest(BufferSize);
  std::uint64_t const RefTimeInterval{8};
  std::uint64_t CurrentRefTime{9};
  std::uint64_t CurrentTime{10};
  std::uint64_t const EventInterval{1};
  for (int i = 0; i < 100; i++) {
    CurrentTime += EventInterval;
    if (CurrentTime > CurrentRefTime + RefTimeInterval) {
      CurrentRefTime += RefTimeInterval;
    }
    TestEvent->Timestamp = CurrentTime;
    UnderTest.addEvent(TestEvent);
    UnderTest.addReferenceTimestamp(CurrentRefTime - 3);
    auto Events = UnderTest.getFrameEvents();
    if (not Events.Events.empty()) {
      ASSERT_LE(Events.RefTime, Events.Events[0].Timestamp);
      ASSERT_NE(BufferSize, Events.Events.size());
      ASSERT_LE((Events.Events.end() - 1)->Timestamp,
                Events.RefTime + RefTimeInterval);
      UnderTest.cullEvents(Events.Events.size());
    }
  }
}
