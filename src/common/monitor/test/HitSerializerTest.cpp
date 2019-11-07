/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/monitor/HitSerializer.h>
#include <test/TestBase.h>

class HitSerializerTest : public TestBase {
  void SetUp() override {  }

  void TearDown() override {  }

protected:
  static const int arraysize = 10000; // max entries
  static const int fboverhead = 106;   // found by experimentation, now with header!
  static const int entrysize = 10;    // three u16 + one u32

  char flatbuffer[1024 * 1024 * 5];


public:
  void copy_buffer(nonstd::span<const uint8_t> b)
  {
    memcpy(flatbuffer, b.data(), b.size_bytes());
  }

};

TEST_F(HitSerializerTest, Constructor) {
  HitSerializer serializer(arraysize, "some_source");
  ASSERT_EQ(0, serializer.getNumEntries());
}

TEST_F(HitSerializerTest, ProduceEmpty) {
  HitSerializer serializer(arraysize, "some_source");
  ASSERT_EQ(0, serializer.getNumEntries());
  auto res = serializer.produce();
  ASSERT_EQ(res, 0);
}

TEST_F(HitSerializerTest, AddEntries) {
  HitSerializer serializer(arraysize, "some_source");
  ASSERT_EQ(0, serializer.getNumEntries());
  for (int i = 1; i < arraysize; i++) {
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_EQ(i, serializer.getNumEntries());
    ASSERT_EQ(res, 0);
  }
  int res = serializer.addEntry(0,0,0,0);
  ASSERT_EQ(0, serializer.getNumEntries());
  ASSERT_EQ(res != 0, true);
}

TEST_F(HitSerializerTest, ManualProduce) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    HitSerializer serializer(maxlen, "some_source");
    auto Produce = [this](auto DataBuffer, auto) {
      this->copy_buffer(DataBuffer);
    };
    serializer.set_callback(Produce);

    ASSERT_EQ(0, serializer.getNumEntries());
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_EQ(res, 0);
    res = serializer.produce();
    EXPECT_EQ(std::string(&flatbuffer[4], 4), "mo01");
    ASSERT_GT(res, 0);
    ASSERT_LE(res, maxlen*entrysize + fboverhead);

    auto deserialized = GetMonitorMessage(flatbuffer);
    EXPECT_EQ(deserialized->source_name()->str(), "some_source");
    EXPECT_EQ(deserialized->data_type(), DataField::MONHit);
  }
}

TEST_F(HitSerializerTest, CheckSmallSizes) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    HitSerializer serializer(maxlen, "some_source");
    for (int i = 1; i < maxlen; i++) {
      int res = serializer.addEntry(0,0,0,0);
      ASSERT_EQ(res, 0);
    }
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_GT(res, 0);
    ASSERT_LE(res, maxlen*entrysize + fboverhead);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
