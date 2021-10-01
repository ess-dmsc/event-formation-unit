/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <gdgem/nmx/TrackSerializer.h>
#include <common/testutils/TestBase.h>

const unsigned int NB_ENTRIES = 256;
const unsigned int BASE_OVERHEAD = 128;
const unsigned int ENTRY_OVERHEAD = 16;

using namespace Gem;

class TrackSerializerTest : public TestBase {
protected:
  Hit e;
  Event event;
  char flatbuffer[100000];

  void addxandy(uint16_t xs, uint16_t xt, uint16_t xa, uint16_t ys, uint16_t yt,
                uint16_t ya) {
    e.coordinate = xs;
    e.time = xt;
    e.weight = xa;
    e.plane = 0;
    event.insert(e);
    e.coordinate = ys;
    e.time = yt;
    e.weight = ya;
    e.plane = 1;
    event.insert(e);
  }
};

TEST_F(TrackSerializerTest, Constructor) {
  TrackSerializer tser(2560, "some_source");
  auto buffer = tser.serialize();
  EXPECT_EQ(buffer.size_bytes(), 0);
  EXPECT_EQ(buffer.data(), nullptr);
}

TEST_F(TrackSerializerTest, AddTrackTooManyHits) {
  int entries = NB_ENTRIES;
  TrackSerializer tser(entries,"some_source");
  for (int i = 0; i < entries + 1; i++) {
    addxandy(i, 2 * i, 500, i - 1, 3 * i - 1, 500);
  }
  EXPECT_FALSE(tser.add_track(event, 0.0, 0.0));
}

TEST_F(TrackSerializerTest, Serialize) {
  unsigned int entries = NB_ENTRIES;
  TrackSerializer tser(entries, "some_source");
  for (unsigned int i = 0; i < entries; i++) {
    addxandy(i, 2 * i, 500, i - 1, 3 * i - 1, 500);
  }
  EXPECT_TRUE(tser.add_track(event, 0.0, 0.0));
  auto buffer = tser.serialize();
  EXPECT_TRUE(buffer.size_bytes() > entries * 2 * 12);
  EXPECT_TRUE(buffer.size_bytes() <
              entries * 2 * 12 + BASE_OVERHEAD + entries * ENTRY_OVERHEAD);
  EXPECT_NE(buffer.data(), nullptr);

  // Ensure header is there
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(&buffer[4]), 4), "mo01");
}

TEST_F(TrackSerializerTest, DeSerialize) {
  unsigned int entries = NB_ENTRIES;
  unsigned int entry_size = 4 * 3; // Three uint32_t's

  TrackSerializer tser(entries, "some_source");
  for (unsigned int i = 0; i < entries; i++) {
    addxandy(i, 0x1111, 0x2222, 100 + i, 0x3333, 0x4444);
  }
  EXPECT_TRUE(tser.add_track(event, 0.0, 0.0));
  EXPECT_EQ(event.ClusterA.hits.size(), entries);
  EXPECT_EQ(event.ClusterB.hits.size(), entries);

  auto buffer = tser.serialize();
  EXPECT_TRUE(buffer.size_bytes() > entries * entry_size * 2); //  x and y
  EXPECT_TRUE(buffer.size_bytes() < entries * entry_size * 2 + BASE_OVERHEAD +
                        entries * ENTRY_OVERHEAD);
  EXPECT_NE(buffer.data(), nullptr);

  memset(flatbuffer, 0, sizeof(flatbuffer));
  memcpy(flatbuffer, buffer.data(), buffer.size_bytes());

  auto monitor = GetMonitorMessage(flatbuffer);
  EXPECT_EQ(monitor->source_name()->str(), "some_source");
  EXPECT_EQ(monitor->data_type(), DataField::GEMTrack);

  auto track = static_cast<const GEMTrack *>(monitor->data());
  auto xdat = track->xtrack();
  auto ydat = track->ytrack();
  EXPECT_EQ(xdat->size(), entries);
  EXPECT_EQ(ydat->size(), entries);

  // Ensure header is there
  EXPECT_EQ(std::string(reinterpret_cast<const char*>(&buffer[4]), 4), "mo01");
}

TEST_F(TrackSerializerTest, Validate1000IncreasingSize) {
  MESSAGE() << "Allocating a TrackSerializer object on every iteration\n";
  for (unsigned int j = 2; j <= 1000; j *= 2) {
    event.ClusterA.hits.clear();
    event.ClusterB.hits.clear();
    unsigned int entries = j;
    unsigned int entry_size = 4 * 3; // Three uint32_t's

    EXPECT_FALSE(event.ClusterA.hits.size());
    EXPECT_FALSE(event.ClusterB.hits.size());

    TrackSerializer tser(entries, "some_source");
    for (unsigned int i = 0; i < entries; i++) {
      addxandy(i, i * 2, i * 3 + 1, entries - i, i * 2 + 0x1000,
               i * 3 + 0x2000);
    }
    EXPECT_TRUE(tser.add_track(event, 0.0, 0.0));
    EXPECT_EQ(event.ClusterA.hits.size(), entries);
    EXPECT_EQ(event.ClusterB.hits.size(), entries);
    auto buffer = tser.serialize();
    // MESSAGE() << "entries: " << entries << ", buffer size: " << buffer.size_bytes() << ",
    // overhead: " << buffer.size_bytes() - entries * entry_size * 2 << "\n";
    EXPECT_TRUE(buffer.size_bytes() > entries * entry_size * 2); //  x and y
    EXPECT_TRUE(buffer.size_bytes() < entries * entry_size * 2 + BASE_OVERHEAD +
                          entries * ENTRY_OVERHEAD);
    EXPECT_NE(buffer.data(), nullptr);

    memcpy(flatbuffer, buffer.data(), buffer.size_bytes());

    auto monitor = GetMonitorMessage(flatbuffer);
    EXPECT_EQ(monitor->source_name()->str(), "some_source");
    EXPECT_EQ(monitor->data_type(), DataField::GEMTrack);

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

    // Ensure header is there
    EXPECT_EQ(std::string(&flatbuffer[4], 4), "mo01");
  }
}

TEST_F(TrackSerializerTest, Validate1000SameSize) {
  unsigned int entries = 256;
  unsigned int entry_size = 4 * 3; // Three uint32_t's
  MESSAGE() << "Reusing the same TrackSerializer object\n";
  TrackSerializer tser(entries, "some_source");
  for (unsigned int i = 1; i <= 1000; i *= 2) {
    event.ClusterA.hits.clear();
    event.ClusterB.hits.clear();
    for (unsigned int j = 0; j < entries; j++) {
      addxandy(j, j * 2, j * 3 + 1, entries - j, j * 2 + 0x1000,
               j * 3 + 0x2000);
    }
    EXPECT_TRUE(tser.add_track(event, 0.0, 0.0));
    auto buffer = tser.serialize();
    // MESSAGE() << "entries: " << entries << ", buffer size: " << buffer.size_bytes() << ",
    // overhead: " << buffer.size_bytes() - entries * entry_size * 2 << "\n";
    EXPECT_TRUE(buffer.size_bytes() > entries * entry_size * 2); //  x and y
    EXPECT_TRUE(buffer.size_bytes() < entries * entry_size * 2 + BASE_OVERHEAD +
                          entries * ENTRY_OVERHEAD);
    EXPECT_NE(buffer.data(), nullptr);

    memcpy(flatbuffer, buffer.data(), buffer.size_bytes());

    auto monitor = GetMonitorMessage(flatbuffer);
    EXPECT_EQ(monitor->source_name()->str(), "some_source");
    EXPECT_EQ(monitor->data_type(), DataField::GEMTrack);

    auto track = static_cast<const GEMTrack *>(monitor->data());
    auto xdat = track->xtrack();
    auto ydat = track->ytrack();
    EXPECT_EQ(xdat->size(), entries);
    EXPECT_EQ(ydat->size(), entries);

    for (unsigned int k = 0; k < entries; k++) {
      EXPECT_EQ((*xdat)[k]->strip(), k);
      EXPECT_EQ((*xdat)[k]->time(), k * 2);
      EXPECT_EQ((*xdat)[k]->adc(), k * 3 + 1);
      EXPECT_EQ((*ydat)[k]->strip(), entries - k);
      EXPECT_EQ((*ydat)[k]->time(), k * 2 + 0x1000);
      EXPECT_EQ((*ydat)[k]->adc(), k * 3 + 0x2000);
    }

    // Ensure header is there
    EXPECT_EQ(std::string(&flatbuffer[4], 4), "mo01");
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
