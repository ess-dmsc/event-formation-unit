/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/FBSerializer.h>
#include <test/TestBase.h>
#include <cstring>

#define GLOBALTIME 0x1000000020000000
#define ARRAYLENGTH 125000

class FBSerializerTest : public TestBase {
    virtual void SetUp() {
      for (int i = 0; i < 200000; i++) {
        tarr[i] = i;
        parr[i] = 200000 - i;
      }
    }

    virtual void TearDown() {}

protected:
  char flatbuffer[1024*1024];
  uint32_t tarr[200000];
  uint32_t parr[200000];
  char * buffer;
  FBSerializer fb{ARRAYLENGTH};
};

TEST_F(FBSerializerTest, Serialize) {
  auto length = fb.serialize(GLOBALTIME, 1, (char *)tarr, (char *)parr, ARRAYLENGTH, &buffer);
  ASSERT_TRUE(length >= ARRAYLENGTH * 8);
  ASSERT_TRUE(length <= ARRAYLENGTH * 8 + 2048);
  ASSERT_TRUE(buffer != 0);
}

TEST_F(FBSerializerTest, SerDeserialize) {
  auto length = fb.serialize(GLOBALTIME, 1, (char *)tarr, (char *)parr, ARRAYLENGTH, &buffer);

  memset(flatbuffer, 0, sizeof(flatbuffer));
  auto events = GetEventMessage(flatbuffer);
  ASSERT_NE(events->message_id(), 1);
  ASSERT_NE(events->pulse_time(), GLOBALTIME);

  memcpy(flatbuffer, buffer, length);
  events = GetEventMessage(flatbuffer);
  ASSERT_EQ(events->message_id(), 1);
  ASSERT_EQ(events->pulse_time(), GLOBALTIME);
}


TEST_F(FBSerializerTest, DeserializeCheckData) {
  auto length = fb.serialize(GLOBALTIME, 1, (char *)tarr, (char *)parr, ARRAYLENGTH, &buffer);

  memcpy(flatbuffer, buffer, length);
  auto events = GetEventMessage(flatbuffer);

  auto detvec = events->detector_id();
  ASSERT_EQ(detvec->size(), ARRAYLENGTH);

  auto timevec = events->time_of_flight();
  ASSERT_EQ(timevec->size(), ARRAYLENGTH);

  for (int i = 0; i < ARRAYLENGTH; i++) {
    ASSERT_EQ((*timevec)[i], i);
    ASSERT_EQ((*detvec)[i], 200000 - i);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
