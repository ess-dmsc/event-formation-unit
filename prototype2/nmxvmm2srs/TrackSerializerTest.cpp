/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/TrackSerializer.h>
#include <test/TestBase.h>
#include <cstring>

#define PLANEX 0
#define PLANEY 1

#define NB_ENTRIES 10

class TrackSerializerTest : public TestBase {
  virtual void SetUp() {}

  virtual void TearDown() {}

protected:
  char * buffer;
  char flatbuffer[100000];
};

TEST_F(TrackSerializerTest, Constructor) {
  TrackSerializer tser(256);
  auto len = tser.serialize(&buffer);
  ASSERT_EQ(len, 0);
  ASSERT_EQ(buffer, nullptr);
}

TEST_F(TrackSerializerTest, IllegalPlane) {
  TrackSerializer tser(256);
  auto res = tser.add_track(PLANEX, 0, 0, 0);
  ASSERT_EQ(res, 1);
  res = tser.add_track(PLANEY, 0, 0, 0);
  ASSERT_EQ(res, 1);
  res = tser.add_track(2, 0, 0, 0);
  ASSERT_EQ(res, -1);
}

TEST_F(TrackSerializerTest, MaxAdds) {
  int entries = NB_ENTRIES;
  TrackSerializer tser(entries);
  for (int i = 0; i < entries; i++) {
    auto res = tser.add_track(PLANEX, i, i*2, i*3);
    ASSERT_EQ(res, i + 1);
    res = tser.add_track(PLANEY, i, i*2, i*3);
    ASSERT_EQ(res, i + 1);
  }
  auto res = tser.add_track(PLANEX, 255, 255*2, 255*3);
  ASSERT_EQ(res, -1);
  res = tser.add_track(PLANEY, 255, 255*2, 255*3);
  ASSERT_EQ(res, -1);
}


TEST_F(TrackSerializerTest, Serialize) {
  int entries = NB_ENTRIES;
  TrackSerializer tser(entries);
  for (int i = 0; i < entries; i++) {
    tser.add_track(PLANEX, i, 2*i, 3*i);
    tser.add_track(PLANEY, i-1, 3*i -1, 2*i -1);
  }
  auto len = tser.serialize(&buffer);
  ASSERT_TRUE(len > entries * 2 * 12);
  ASSERT_TRUE(len < entries * 2 * 12 + 256);
  MESSAGE() << len << "\n";
  ASSERT_TRUE(buffer != nullptr);
}

TEST_F(TrackSerializerTest, DeSerialize) {
  int entries = NB_ENTRIES;
  int entry_size = 4 * 3; // Three uint32_t's

  TrackSerializer tser(entries);
  for (int i = 0; i < entries; i++) {
    tser.add_track(PLANEX, i, 0x11111111, 0x22222222);
    tser.add_track(PLANEY, 100 + i, 0x33333333, 0x44444444);
  }
  auto len = tser.serialize(&buffer);
  MESSAGE() << len << "\n";
  ASSERT_TRUE(len > entries * entry_size * 2); //  x and y
  ASSERT_TRUE(len < entries * entry_size * 2 + 512);
  ASSERT_TRUE(buffer != nullptr);

  memset(flatbuffer, 0, sizeof(flatbuffer));
  memcpy(flatbuffer, buffer, len);

  auto monitor = GetMonitorMessage(flatbuffer);
  auto dtype = monitor->data_type();
  ASSERT_EQ(dtype, DataField_GEMTrack);

  auto track = static_cast<const GEMTrack*>(monitor->data());
  auto xdat = track->xtrack();
  auto ydat = track->ytrack();
  ASSERT_EQ(xdat->size(), entries);
  ASSERT_EQ(ydat->size(), entries);
}

TEST_F(TrackSerializerTest, Validate1000IncreasingSize) {
  MESSAGE() << "Allocating a TrackSerializer object on every iteration\n";
  for (int i = 1; i <= 1000; i++) {
    int entries = i;
    int entry_size = 4 * 3; // Three uint32_t's

    TrackSerializer tser(entries);
    for (int i = 0; i < entries; i++) {
      tser.add_track(PLANEX, i, i*2, i*3);
      tser.add_track(PLANEY, entries - i, i*2 + 0x10000000, i*3 + 0x20000000);
    }
    auto len = tser.serialize(&buffer);
    //MESSAGE() << "entries: " << entries << ", buffer size: " << len << ", overhead: " << len - entries * entry_size * 2 << "\n";
    ASSERT_TRUE(len > entries * entry_size * 2); //  x and y
    ASSERT_TRUE(len < entries * entry_size * 2 + 16 * entries + 84);
    ASSERT_TRUE(buffer != nullptr);

    memcpy(flatbuffer, buffer, len);

    auto monitor = GetMonitorMessage(flatbuffer);
    auto dtype = monitor->data_type();
    ASSERT_EQ(dtype, DataField_GEMTrack);

    auto track = static_cast<const GEMTrack*>(monitor->data());
    auto xdat = track->xtrack();
    auto ydat = track->ytrack();
    ASSERT_EQ(xdat->size(), entries);
    ASSERT_EQ(ydat->size(), entries);

    for (int i = 0; i < entries; i++) {
      ASSERT_EQ((*xdat)[i]->strip(), i);
      ASSERT_EQ((*xdat)[i]->time(), i*2);
      ASSERT_EQ((*xdat)[i]->adc(),  i*3);
      ASSERT_EQ((*ydat)[i]->strip(), entries - i);
      ASSERT_EQ((*ydat)[i]->time(), i*2 + 0x10000000);
      ASSERT_EQ((*ydat)[i]->adc(),  i*3 + 0x20000000);
    }
  }
}

TEST_F(TrackSerializerTest, Validate1000SameSize) {
  int entries = 256;
  int entry_size = 4 * 3; // Three uint32_t's
  MESSAGE() << "Reusing the same TrackSerializer object\n";
  TrackSerializer tser(entries);
  for (int i = 1; i <= 1000; i++) {
    for (int i = 0; i < entries; i++) {
      tser.add_track(PLANEX, i, i*2, i*3);
      tser.add_track(PLANEY, entries - i, i*2 + 0x10000000, i*3 + 0x20000000);
    }
    auto len = tser.serialize(&buffer);
    //MESSAGE() << "entries: " << entries << ", buffer size: " << len << ", overhead: " << len - entries * entry_size * 2 << "\n";
    ASSERT_TRUE(len > entries * entry_size * 2); //  x and y
    ASSERT_TRUE(len < entries * entry_size * 2 + 16 * entries + 84);
    ASSERT_TRUE(buffer != nullptr);

    memcpy(flatbuffer, buffer, len);

    auto monitor = GetMonitorMessage(flatbuffer);
    auto dtype = monitor->data_type();
    ASSERT_EQ(dtype, DataField_GEMTrack);

    auto track = static_cast<const GEMTrack*>(monitor->data());
    auto xdat = track->xtrack();
    auto ydat = track->ytrack();
    ASSERT_EQ(xdat->size(), entries);
    ASSERT_EQ(ydat->size(), entries);

    for (int i = 0; i < entries; i++) {
      ASSERT_EQ((*xdat)[i]->strip(), i);
      ASSERT_EQ((*xdat)[i]->time(), i*2);
      ASSERT_EQ((*xdat)[i]->adc(),  i*3);
      ASSERT_EQ((*ydat)[i]->strip(), entries - i);
      ASSERT_EQ((*ydat)[i]->time(), i*2 + 0x10000000);
      ASSERT_EQ((*ydat)[i]->adc(),  i*3 + 0x20000000);
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
