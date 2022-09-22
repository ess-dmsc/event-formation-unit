/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <multigrid/mesytec/Sis3153Parser.h>
#include <multigrid/mesytec/Vmmr16Parser.h>
#include <multigrid/mesytec/test/TestData.h>

using namespace Multigrid;

class Vmmr16ParserTest : public TestBase {
protected:
  Sis3153Parser sis;
  VMMR16Parser vmmr;
  void SetUp() override {}
  void TearDown() override {}
};

// \todo test default state

TEST_F(Vmmr16ParserTest, ErrNoTimeStamp) {
  auto res = sis.parse(err_no_timestamp);
  ASSERT_EQ(res, 0);

  EXPECT_EQ(vmmr.parse(sis.buffers.front()), 528);

  // \todo ensure strong guarantee?
}

TEST_F(Vmmr16ParserTest, ParseRecordedWSData) {
  ASSERT_EQ(sis.parse(ws1), 0);

  auto res = vmmr.parse(sis.buffers.front());

  EXPECT_EQ(res, 0);
  EXPECT_EQ(vmmr.converted_data.size(), 128);
  EXPECT_EQ(vmmr.trigger_count(), 1);
  EXPECT_FALSE(vmmr.externalTrigger());
  EXPECT_EQ(vmmr.time(), 69134374606);
}

TEST_F(Vmmr16ParserTest, ParseRecordedWSDataII) {
  ASSERT_EQ(sis.parse(ws2), 0);

  auto res = vmmr.parse(sis.buffers.front());
  EXPECT_EQ(res, 0);
  EXPECT_EQ(vmmr.converted_data.size(), 128);
  EXPECT_EQ(vmmr.trigger_count(), 1);
  EXPECT_FALSE(vmmr.externalTrigger());
  EXPECT_EQ(vmmr.time(), 68719698574);

  auto res2 = vmmr.parse(sis.buffers.back());
  EXPECT_EQ(res2, 0);
  EXPECT_EQ(vmmr.converted_data.size(), 128);
  EXPECT_EQ(vmmr.trigger_count(), 2);
  EXPECT_FALSE(vmmr.externalTrigger());
  EXPECT_EQ(vmmr.time(), 68719711681);
}

TEST_F(Vmmr16ParserTest, ParseRecordedWSDataIII) {
  ASSERT_EQ(sis.parse(ws3), 0);

  size_t total_err{0};
  size_t total_parsed{0};
  for (const auto &b : sis.buffers) {
    total_err += vmmr.parse(b);
    total_parsed += vmmr.converted_data.size();
  }

  EXPECT_EQ(total_err, 0);
  EXPECT_EQ(total_parsed, 256);
  EXPECT_EQ(vmmr.trigger_count(), 4);
}

TEST_F(Vmmr16ParserTest, ParseRecordedWSDataMultipleTriggers) {
  ASSERT_EQ(sis.parse(ws4), 0);

  size_t total_err{0};
  size_t total_parsed{0};
  for (const auto &b : sis.buffers) {
    total_err += vmmr.parse(b);
    total_parsed += vmmr.converted_data.size();
  }

  EXPECT_EQ(total_err, 0);
  EXPECT_EQ(total_parsed, 54);
  EXPECT_EQ(vmmr.trigger_count(), 36);
}

// \todo external trigger

// \todo spoof high time

// \todo EventData1

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
