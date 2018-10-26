/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <gdgem/nmx/Event.h>
#include <gdgem/nmx/Hit.h>
#include <gdgem/nmx/TrackSerializer.h>
#include <test/TestBase.h>

const unsigned int NB_ENTRIES = 256;
const unsigned int BASE_OVERHEAD = 128;
const unsigned int ENTRY_OVERHEAD = 16;

class TrackSerializerTest : public TestBase {
protected:
  Hit e;
  Event event;
  char flatbuffer[100000];

  void addxandy(uint16_t xs, uint16_t xt, uint16_t xa, uint16_t ys, uint16_t yt,
                uint16_t ya) {
    e.strip = xs;
    e.time = xt;
    e.adc = xa;
    e.plane_id = 0;
    event.insert_hit(e);
    e.strip = ys;
    e.time = yt;
    e.adc = ya;
    e.plane_id = 1;
    event.insert_hit(e);
  }
};

TEST_F(TrackSerializerTest, Constructor) {
  TrackSerializer tser(2560, 0, 1);
  auto buffer = tser.serialize();
  EXPECT_EQ(buffer.size, 0);
  EXPECT_EQ(buffer.address, nullptr);
}

TEST_F(TrackSerializerTest, AddTrackTooFewHits) {
  int entries = NB_ENTRIES;
  TrackSerializer tser(entries, 1, 1);
  EXPECT_FALSE(tser.add_track(event));
}

TEST_F(TrackSerializerTest, AddTrackTooManyHits) {
  int entries = NB_ENTRIES;
  TrackSerializer tser(entries, 0, 1);
  for (int i = 0; i < entries + 1; i++) {
    addxandy(i, 2 * i, 500, i - 1, 3 * i - 1, 500);
  }
  EXPECT_FALSE(tser.add_track(event));
}

TEST_F(TrackSerializerTest, Serialize) {
  unsigned int entries = NB_ENTRIES;
  TrackSerializer tser(entries, 0, 1);
  for (unsigned int i = 0; i < entries; i++) {
    addxandy(i, 2 * i, 500, i - 1, 3 * i - 1, 500);
  }
  EXPECT_TRUE(tser.add_track(event));
  auto buffer = tser.serialize();
  EXPECT_TRUE(buffer.size > entries * 2 * 12);
  EXPECT_TRUE(buffer.size <
              entries * 2 * 12 + BASE_OVERHEAD + entries * ENTRY_OVERHEAD);
  EXPECT_NE(buffer.address, nullptr);
}

TEST_F(TrackSerializerTest, DeSerialize) {
  unsigned int entries = NB_ENTRIES;
  unsigned int entry_size = 4 * 3; // Three uint32_t's

  TrackSerializer tser(entries, 0, 1);
  for (unsigned int i = 0; i < entries; i++) {
    addxandy(i, 0x1111, 0x2222, 100 + i, 0x3333, 0x4444);
  }
  EXPECT_TRUE(tser.add_track(event));
  EXPECT_EQ(event.x.entries.size(), entries);
  EXPECT_EQ(event.y.entries.size(), entries);

  auto buffer = tser.serialize();
  EXPECT_TRUE(buffer.size > entries * entry_size * 2); //  x and y
  EXPECT_TRUE(buffer.size < entries * entry_size * 2 + BASE_OVERHEAD +
                        entries * ENTRY_OVERHEAD);
  EXPECT_NE(buffer.address, nullptr);

  memset(flatbuffer, 0, sizeof(flatbuffer));
  memcpy(flatbuffer, buffer.address, buffer.size);

  auto monitor = GetMonitorMessage(flatbuffer);
  auto dtype = monitor->data_type();
  EXPECT_EQ(dtype, DataField::GEMTrack);

  auto track = static_cast<const GEMTrack *>(monitor->data());
  auto xdat = track->xtrack();
  auto ydat = track->ytrack();
  EXPECT_EQ(xdat->size(), entries);
  EXPECT_EQ(ydat->size(), entries);
}

TEST_F(TrackSerializerTest, Validate1000IncreasingSize) {
  MESSAGE() << "Allocating a TrackSerializer object on every iteration\n";
  for (unsigned int j = 2; j <= 1000; j *= 2) {
    event.x.entries.clear();
    event.y.entries.clear();
    unsigned int entries = j;
    unsigned int entry_size = 4 * 3; // Three uint32_t's

    EXPECT_FALSE(event.x.entries.size());
    EXPECT_FALSE(event.y.entries.size());

    TrackSerializer tser(entries, 0, 1);
    for (unsigned int i = 0; i < entries; i++) {
      addxandy(i, i * 2, i * 3 + 1, entries - i, i * 2 + 0x1000,
               i * 3 + 0x2000);
    }
    EXPECT_TRUE(tser.add_track(event));
    EXPECT_EQ(event.x.entries.size(), entries);
    EXPECT_EQ(event.y.entries.size(), entries);
    auto buffer = tser.serialize();
    // MESSAGE() << "entries: " << entries << ", buffer size: " << buffer.size << ",
    // overhead: " << buffer.size - entries * entry_size * 2 << "\n";
    EXPECT_TRUE(buffer.size > entries * entry_size * 2); //  x and y
    EXPECT_TRUE(buffer.size < entries * entry_size * 2 + BASE_OVERHEAD +
                          entries * ENTRY_OVERHEAD);
    EXPECT_NE(buffer.address, nullptr);

    memcpy(flatbuffer, buffer.address, buffer.size);

    auto monitor = GetMonitorMessage(flatbuffer);
    auto dtype = monitor->data_type();
    EXPECT_EQ(dtype, DataField::GEMTrack);

    auto track = static_cast<const GEMTrack *>(monitor->data());
    auto xdat = track->xtrack();
    auto ydat = track->ytrack();
    EXPECT_EQ(xdat->size(), entries);
    EXPECT_EQ(ydat->size(), entries);

    for (unsigned int i = 0; i < entries; i++) {
      EXPECT_EQ((*xdat)[i]->strip(), i);
      EXPECT_EQ((*xdat)[i]->time(), i * 2);
      EXPECT_EQ((*xdat)[i]->adc(), i * 3 + 1);
      EXPECT_EQ((*ydat)[i]->strip(), entries - i);
      EXPECT_EQ((*ydat)[i]->time(), i * 2 + 0x1000);
      EXPECT_EQ((*ydat)[i]->adc(), i * 3 + 0x2000);
    }
  }
}

TEST_F(TrackSerializerTest, Validate1000SameSize) {
  unsigned int entries = 256;
  unsigned int entry_size = 4 * 3; // Three uint32_t's
  MESSAGE() << "Reusing the same TrackSerializer object\n";
  TrackSerializer tser(entries, 0, 1);
  for (unsigned int i = 1; i <= 1000; i *= 2) {
    event.x.entries.clear();
    event.y.entries.clear();
    for (unsigned int i = 0; i < entries; i++) {
      addxandy(i, i * 2, i * 3 + 1, entries - i, i * 2 + 0x1000,
               i * 3 + 0x2000);
    }
    EXPECT_TRUE(tser.add_track(event));
    auto buffer = tser.serialize();
    // MESSAGE() << "entries: " << entries << ", buffer size: " << buffer.size << ",
    // overhead: " << buffer.size - entries * entry_size * 2 << "\n";
    EXPECT_TRUE(buffer.size > entries * entry_size * 2); //  x and y
    EXPECT_TRUE(buffer.size < entries * entry_size * 2 + BASE_OVERHEAD +
                          entries * ENTRY_OVERHEAD);
    EXPECT_NE(buffer.address, nullptr);

    memcpy(flatbuffer, buffer.address, buffer.size);

    auto monitor = GetMonitorMessage(flatbuffer);
    auto dtype = monitor->data_type();
    EXPECT_EQ(dtype, DataField::GEMTrack);

    auto track = static_cast<const GEMTrack *>(monitor->data());
    auto xdat = track->xtrack();
    auto ydat = track->ytrack();
    EXPECT_EQ(xdat->size(), entries);
    EXPECT_EQ(ydat->size(), entries);

    for (unsigned int i = 0; i < entries; i++) {
      EXPECT_EQ((*xdat)[i]->strip(), i);
      EXPECT_EQ((*xdat)[i]->time(), i * 2);
      EXPECT_EQ((*xdat)[i]->adc(), i * 3 + 1);
      EXPECT_EQ((*ydat)[i]->strip(), entries - i);
      EXPECT_EQ((*ydat)[i]->time(), i * 2 + 0x1000);
      EXPECT_EQ((*ydat)[i]->adc(), i * 3 + 0x2000);
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
