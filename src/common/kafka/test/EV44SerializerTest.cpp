// Copyright (C) 2022 European Spallation Source ERIC

#include "ev44_events_generated.h"
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/Producer.h>
#include <common/testutils/TestBase.h>
#include <cstring>

//#define ARRAYLENGTH 125000
#define ARRAYLENGTH 10

struct MockProducer {
  inline void produce(nonstd::span<const uint8_t>, int64_t) { NumberOfCalls++; }

  size_t NumberOfCalls{0};
};

class EV44SerializerTest : public TestBase {
  void SetUp() override {
    for (int i = 0; i < 200000; i++) {
      time[i] = i;
      pixel[i] = 200000 - i;
    }
  }

  void TearDown() override {}

protected:
  char flatbuffer[1024 * 1024];
  uint32_t time[200000];
  uint32_t pixel[200000];
  EV44Serializer fb{ARRAYLENGTH, "nameless"};
};

TEST_F(EV44SerializerTest, Serialize) {
  for (size_t i = 0; i < ARRAYLENGTH; i++)
    fb.addEvent(i, i);
  auto buffer = fb.serialize();
  EXPECT_GE(buffer.size_bytes(), ARRAYLENGTH * 8);
  EXPECT_LE(buffer.size_bytes(), ARRAYLENGTH * 8 + 2048);
  ASSERT_TRUE(not buffer.empty());

  EXPECT_EQ(std::string(reinterpret_cast<const char *>(&buffer[4]), 4), "ev44");
}

TEST_F(EV44SerializerTest, SerDeserialize) {
  for (size_t i = 0; i < ARRAYLENGTH - 1; i++)
    fb.addEvent(i, i);
  auto buffer = fb.serialize();

  memset(flatbuffer, 0, sizeof(flatbuffer));
  memcpy(flatbuffer, buffer.data(), buffer.size_bytes());

  EXPECT_EQ(std::string(&flatbuffer[4], 4), "ev44");
  auto events = GetEvent44Message(flatbuffer);
  EXPECT_EQ(events->source_name()->str(), "nameless");
  // ASSERT_EQ(events->message_id(), 1);
}

TEST_F(EV44SerializerTest, SerPulseTime) {
  fb.setReferenceTime(12345);
  ASSERT_EQ(fb.referenceTime(), 12345);
  auto buffer = fb.serialize();

  memset(flatbuffer, 0, sizeof(flatbuffer));

  // auto events = GetEvent44Message(flatbuffer);
  ///\todo check contents of reference_time, it is a flatbuffers vector
  // ASSERT_NE(events->reference_time()->begin()->, 12345);

  memcpy(flatbuffer, buffer.data(), buffer.size_bytes());
  EXPECT_EQ(std::string(&flatbuffer[4], 4), "ev44");
  // events = GetEvent44Message(flatbuffer);
  ///\todo, work out why this test fails when all other source_name tests pass
  // EXPECT_EQ(events->source_name()->str(), "nameless");
  // ASSERT_EQ(events->reference_time()->front, 12345);
}

TEST_F(EV44SerializerTest, DeserializeCheckData) {
  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    auto len = fb.addEvent(time[i], pixel[i]);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(fb.eventCount(), i + 1);
  }

  auto buffer = fb.serialize();
  ASSERT_TRUE(not buffer.empty());

  memcpy(flatbuffer, buffer.data(), buffer.size_bytes());
  EXPECT_EQ(std::string(&flatbuffer[4], 4), "ev44");

  auto veri = flatbuffers::Verifier((uint8_t *)flatbuffer, buffer.size_bytes());
  ASSERT_TRUE(VerifyEvent44MessageBuffer(veri));
  auto events = GetEvent44Message(flatbuffer);
  EXPECT_EQ(events->source_name()->str(), "nameless");

  auto detvec = events->pixel_id();
  EXPECT_EQ(detvec->size(), ARRAYLENGTH - 1);

  auto timevec = events->time_of_flight();
  EXPECT_EQ(timevec->size(), ARRAYLENGTH - 1);

  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    EXPECT_EQ((*timevec)[i], i);
    EXPECT_EQ((*detvec)[i], 200000 - i);
  }

  EXPECT_EQ(events->source_name()->str(), "nameless");
}

TEST_F(EV44SerializerTest, AutoDeserialize) {
  MockProducer mp;
  auto Produce = [&mp](auto A, auto B) { mp.produce(A, B); };
  fb.setProducerCallback(Produce);

  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    auto len = fb.addEvent(time[i], pixel[i]);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(fb.eventCount(), i + 1);
  }
  EXPECT_EQ(mp.NumberOfCalls, 0);

  auto len = fb.addEvent(time[ARRAYLENGTH - 1], pixel[ARRAYLENGTH - 1]);
  EXPECT_GT(len, 0);
  EXPECT_EQ(mp.NumberOfCalls, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
