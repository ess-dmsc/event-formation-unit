/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <cstring>
#include <test/TestBase.h>

//#define ARRAYLENGTH 125000
#define ARRAYLENGTH 10

class EV42SerializerTest : public TestBase {
  virtual void SetUp() {
    for (int i = 0; i < 200000; i++) {
      time[i] = i;
      pixel[i] = 200000 - i;
    }
  }

  virtual void TearDown() {}

protected:
  char flatbuffer[1024 * 1024];
  uint32_t time[200000];
  uint32_t pixel[200000];
  EV42Serializer fb{ARRAYLENGTH, "nameless"};
};

TEST_F(EV42SerializerTest, Serialize) {
  for (size_t i=0; i < ARRAYLENGTH; i++)
    fb.addevent(i,i);
  auto buffer = fb.serialize();
  ASSERT_TRUE(buffer.size >= ARRAYLENGTH * 8);
  ASSERT_TRUE(buffer.size <= ARRAYLENGTH * 8 + 2048);
  ASSERT_TRUE(buffer.buffer != nullptr);
}

TEST_F(EV42SerializerTest, SerDeserialize) {
  for (size_t i=0; i < ARRAYLENGTH-1; i++)
    fb.addevent(i,i);
  auto buffer = fb.serialize();

  memset(flatbuffer, 0, sizeof(flatbuffer));
  auto events = GetEventMessage(flatbuffer);
  ASSERT_NE(events->message_id(), 1);

  memcpy(flatbuffer, buffer.buffer, buffer.size);
  events = GetEventMessage(flatbuffer);
  ASSERT_EQ(events->message_id(), 1);
}

TEST_F(EV42SerializerTest, SerPulseTime) {
  fb.set_pulse_time(12345);
  ASSERT_EQ(fb.get_pulse_time(), 12345);
  auto buffer = fb.serialize();

  memset(flatbuffer, 0, sizeof(flatbuffer));
  auto events = GetEventMessage(flatbuffer);
  ASSERT_NE(events->pulse_time(), 12345);

  memcpy(flatbuffer, buffer.buffer, buffer.size);
  events = GetEventMessage(flatbuffer);
  ASSERT_EQ(events->pulse_time(), 12345);
}

TEST_F(EV42SerializerTest, DeserializeCheckData) {
  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    auto len = fb.addevent(time[i], pixel[i]);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(fb.events(), i+1);
  }

  auto buffer = fb.serialize();
  ASSERT_TRUE(buffer.size > 0);
  ASSERT_TRUE(buffer.buffer != nullptr);

  memcpy(flatbuffer, buffer.buffer, buffer.size);
  auto veri = flatbuffers::Verifier((uint8_t *)flatbuffer, buffer.size);
  ASSERT_TRUE(VerifyEventMessageBuffer(veri));
  auto events = GetEventMessage(flatbuffer);

  auto detvec = events->detector_id();
  EXPECT_EQ(detvec->size(), ARRAYLENGTH - 1);

  auto timevec = events->time_of_flight();
  EXPECT_EQ(timevec->size(), ARRAYLENGTH - 1);

  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    EXPECT_EQ((*timevec)[i], i);
    EXPECT_EQ((*detvec)[i], 200000 - i);
  }
}

TEST_F(EV42SerializerTest, AutoDeserialize) {
  for (int i = 0; i < ARRAYLENGTH - 1; i++) {
    auto len = fb.addevent(time[i], pixel[i]);
    ASSERT_EQ(len, 0);
    ASSERT_EQ(fb.events(), i + 1);
  }

  auto len = fb.addevent(time[ARRAYLENGTH - 1], pixel[ARRAYLENGTH - 1]);
  ASSERT_TRUE(len > 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
