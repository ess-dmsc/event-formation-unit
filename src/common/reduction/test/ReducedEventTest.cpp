/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/NeutronEvent.h>
#include <common/reduction/ReducedEvent.h>

#include <common/testutils/TestBase.h>

class ReducedEventTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ReducedEventTest, DefaultReducedHitIsBad) {
  ReducedHit hit;
  EXPECT_FALSE(hit.is_center_good());
}

TEST_F(ReducedEventTest, HitRoundedCenter) {
  ReducedHit hit;
  hit.center = 2.7;
  EXPECT_TRUE(hit.is_center_good());
  EXPECT_EQ(hit.center_rounded(), 3.0);
}

TEST_F(ReducedEventTest, PrintHit) {
  ReducedHit hit;
  hit.center = 2.7;
  EXPECT_FALSE(hit.to_string().empty());
  GTEST_COUT << "NOT A UNIT TEST: please manually check output\n";
  GTEST_COUT << hit.to_string() << "\n";
}

TEST_F(ReducedEventTest, DefaultReducedEventIsBad) {
  ReducedEvent event;
  EXPECT_FALSE(event.good);
}

TEST_F(ReducedEventTest, PrintEvent) {
  ReducedEvent event;
  event.x.center = 2.7;
  EXPECT_FALSE(event.to_string().empty());
  GTEST_COUT << "NOT A UNIT TEST: please manually check output\n";
  GTEST_COUT << event.to_string() << "\n";
  GTEST_COUT << event.to_string_simple() << "\n";
}

TEST_F(ReducedEventTest, PrintNeutronEvent) {
  NeutronEvent event;
  EXPECT_FALSE(event.to_string().empty());
  GTEST_COUT << "NOT A UNIT TEST: please manually check output\n";
  GTEST_COUT << event.to_string() << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
