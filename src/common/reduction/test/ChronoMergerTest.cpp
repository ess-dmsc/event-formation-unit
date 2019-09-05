/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/ChronoMerger.h>
#include <test/TestBase.h>

class ChronoMergerTest : public TestBase {
protected:
  ChronoMerger merger{100, 3};

  void add_events(size_t module, uint64_t time_start, uint64_t time_end) {
    for (uint64_t t = time_start; t <= time_end; ++t)
      merger.insert(module, {t, 0});
  }

  void SetUp() override { }
  void TearDown() override { }
};

TEST_F(ChronoMergerTest, BadModule) {
  EXPECT_NO_THROW(merger.insert(0, {0,0}));
  EXPECT_NO_THROW(merger.insert(1, {0,0}));
  EXPECT_NO_THROW(merger.insert(2, {0,0}));
  EXPECT_ANY_THROW(merger.insert(3, {0,0}));
  EXPECT_ANY_THROW(merger.insert(4, {0,0}));
}

TEST_F(ChronoMergerTest, Empty) {
  EXPECT_TRUE(merger.empty());
  merger.insert(0, {0,0});
  EXPECT_FALSE(merger.empty());
  merger.pop_earliest();
  EXPECT_TRUE(merger.empty());
}

TEST_F(ChronoMergerTest, PopEarliest) {
  merger.insert(0, {1,2});
  merger.insert(0, {3,4});
  auto n1 = merger.pop_earliest();
  EXPECT_EQ(n1.time, 1);
  EXPECT_EQ(n1.pixel_id, 2);
  auto n2 = merger.pop_earliest();
  EXPECT_EQ(n2.time, 3);
  EXPECT_EQ(n2.pixel_id, 4);
  EXPECT_TRUE(merger.empty());
}

TEST_F(ChronoMergerTest, Horizon) {
  EXPECT_EQ(merger.horizon(), 0);
  merger.insert(0, {5,0});
  EXPECT_EQ(merger.horizon(), 0);
  merger.insert(1, {4,0});
  EXPECT_EQ(merger.horizon(), 0);
  merger.insert(2, {3,0});
  EXPECT_EQ(merger.horizon(), 3);
  merger.insert(2, {6,0});
  EXPECT_EQ(merger.horizon(), 4);
  merger.insert(1, {7,0});
  EXPECT_EQ(merger.horizon(), 5);
  merger.insert(0, {8,0});
  EXPECT_EQ(merger.horizon(), 6);
}

TEST_F(ChronoMergerTest, Earliest) {
  merger.insert(0, {5,0});
  merger.insert(1, {4,0});
  merger.insert(2, {3,0});
  merger.insert(2, {6,0});
  merger.insert(1, {7,0});
  merger.insert(0, {8,0});

  EXPECT_EQ(merger.earliest(), 5);
  merger.sort();

  EXPECT_EQ(merger.earliest(), 3);
  merger.pop_earliest();
  EXPECT_EQ(merger.earliest(), 4);
  merger.pop_earliest();
  EXPECT_EQ(merger.earliest(), 5);
  merger.pop_earliest();
  EXPECT_EQ(merger.earliest(), 6);
  merger.pop_earliest();
  EXPECT_EQ(merger.earliest(), 7);
  merger.pop_earliest();
  EXPECT_EQ(merger.earliest(), 8);
  merger.pop_earliest();
  EXPECT_TRUE(merger.empty());
}

TEST_F(ChronoMergerTest, Ready) {
  EXPECT_FALSE(merger.ready());
  merger.insert(0, {3,0});
  EXPECT_FALSE(merger.ready());
  merger.insert(1, {4,0});
  EXPECT_FALSE(merger.ready());
  merger.insert(2, {5,0});
  EXPECT_FALSE(merger.ready());

  merger.insert(0, {104,0});
  EXPECT_FALSE(merger.ready());
  merger.insert(1, {105,0});
  EXPECT_FALSE(merger.ready());
  merger.insert(2, {106,0});
  EXPECT_TRUE(merger.ready());

  merger.pop_earliest();
  EXPECT_FALSE(merger.ready());

  merger.insert(0, {105,0});
  merger.sort();
  EXPECT_TRUE(merger.ready());
  merger.pop_earliest();
  EXPECT_FALSE(merger.ready());

  merger.insert(0, {106,0});
  merger.insert(1, {106,0});
  merger.sort();
  EXPECT_TRUE(merger.ready());
  merger.pop_earliest();
  EXPECT_FALSE(merger.ready());
}

TEST_F(ChronoMergerTest, Reset) {
  merger.insert(0, {5,0});
  merger.insert(1, {4,0});
  merger.insert(2, {3,0});
  merger.insert(2, {6,0});
  merger.insert(1, {7,0});
  merger.insert(0, {8,0});
  EXPECT_EQ(merger.horizon(), 6);
  while (!merger.empty())
    merger.pop_earliest();
  EXPECT_TRUE(merger.empty());
  EXPECT_EQ(merger.horizon(), 6);
  merger.reset();
  EXPECT_EQ(merger.horizon(), 0);
}


TEST_F(ChronoMergerTest, Print) {
  merger.insert(0, {3,0});
  merger.insert(1, {4,0});
  merger.insert(2, {5,0});
  merger.insert(0, {104,0});
  merger.insert(1, {105,0});
  merger.insert(2, {106,0});
  merger.insert(0, {105,0});
  merger.insert(0, {106,0});
  merger.insert(1, {106,0});
  merger.sort();

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "SIMPLE:\n" << merger.debug("  ", false);
  MESSAGE() << "VERBOSE:\n" << merger.debug("  ", true);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
