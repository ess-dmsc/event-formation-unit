/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/ReadoutSerializer.h>
#include <common/Producer.h>
#include <test/TestBase.h>

class ReadoutSerializerTest : public TestBase {
  virtual void SetUp() {  }

  virtual void TearDown() {  }

protected:
  static const int arraysize = 10000; // max entries
  static const int fboverhead = 82;   // found by experimentation
  static const int entrysize = 10;    // three u16 + one u32
  Producer producer{"localhost:9092", "NMX_monitor"};
};

TEST_F(ReadoutSerializerTest, Constructor) {
  ReadoutSerializer serializer(arraysize, producer);
  ASSERT_EQ(0, serializer.getNumEntries());
}

TEST_F(ReadoutSerializerTest, AddEntries) {
  ReadoutSerializer serializer(arraysize, producer);
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

TEST_F(ReadoutSerializerTest, ManualProduce) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    ReadoutSerializer serializer(maxlen, producer);
    ASSERT_EQ(0, serializer.getNumEntries());
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_EQ(res, 0);
    res = serializer.produce();
    ASSERT_TRUE(res > 0);
    ASSERT_TRUE(res <= maxlen*entrysize + fboverhead);
  }
}

TEST_F(ReadoutSerializerTest, CheckSmallSizes) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    ReadoutSerializer serializer(maxlen, producer);
    for (int i = 1; i < maxlen; i++) {
      int res = serializer.addEntry(0,0,0,0);
      ASSERT_EQ(res, 0);
    }
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_TRUE(res > 0);
    ASSERT_TRUE(res <= maxlen*entrysize + fboverhead);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
