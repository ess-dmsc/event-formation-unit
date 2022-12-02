// Copyright (C) 2016, 2017 European Spallation Source ERIC

#include <common/testutils/TestBase.h>

#include <common/reduction/analysis/MgAnalyzer.h>

class MgAnalyzerTest : public TestBase {
protected:
  Hit hit;
  Event event;
  MGAnalyzer analyzer;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MgAnalyzerTest, AnalyzeInvalid) {
  auto result = analyzer.analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(MgAnalyzerTest, AnalyzeBadY) {
  hit.weight = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);

  EXPECT_FALSE(result.good);
}

TEST_F(MgAnalyzerTest, AnalyzeBadX) {
  hit.plane = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(MgAnalyzerTest, AnalyzeGood) {
  hit.weight = 1;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  auto result = analyzer.analyze(event);
  EXPECT_TRUE(result.good);
}

TEST_F(MgAnalyzerTest, InsertInvalid) {
  hit.weight = 1;
  hit.plane = 0;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  hit.plane = 2;
  event.insert(hit);
  EXPECT_EQ(2, event.totalHitCount());
}

TEST_F(MgAnalyzerTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  auto result = analyzer.analyze(event);
  MESSAGE() << result.to_string() << "\n";
}

/// \todo more & better tests of this required

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
