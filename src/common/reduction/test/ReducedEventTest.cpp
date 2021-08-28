/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/ReducedEvent.h>
#include <common/reduction/NeutronEvent.h>

#include <test/TestBase.h>

class ReducedEventTest : public TestBase {
protected:
  void SetUp() override { }
  void TearDown() override { }
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
  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << hit.to_string() << "\n";
}

TEST_F(ReducedEventTest, DefaultReducedEventIsBad) {
  ReducedEvent event;
  EXPECT_FALSE(event.good);
}

TEST_F(ReducedEventTest, PrintEvent) {
  ReducedEvent event;
  event.x.center = 2.7;
  EXPECT_FALSE(event.to_string().empty());
  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << event.to_string() << "\n";
}

TEST_F(ReducedEventTest, PrintNeutronEvent) {
  NeutronEvent event;
  EXPECT_FALSE(event.to_string().empty());
  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << event.to_string() << "\n";
  MESSAGE() << event.to_string_simple() << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
