/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/CdtFile.h>
#include <test/TestBase.h>

using namespace Jalousie;

class CdtFileTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CdtFileTest, SurveyNoiseFile) {
  CdtFile file(TEST_DATA_PATH "noise.bin");
  EXPECT_EQ(file.survey_results.unidentified_board, 0);
  EXPECT_EQ(file.survey_results.events_found, 249770);
  EXPECT_EQ(file.survey_results.metadata_found, 5814);
  EXPECT_EQ(file.survey_results.adc_found, 0);
  EXPECT_EQ(file.survey_results.chopper_pulses, 113);
  EXPECT_EQ(file.survey_results.board_blocks, 5701);
}

TEST_F(CdtFileTest, OutputMatchesSurvey) {
  CdtFile file(TEST_DATA_PATH "noise.bin");
  size_t count{0};
  while( file.read() ) {
    count += file.Data.size();
  }
  EXPECT_EQ(count, file.count());
}

TEST_F(CdtFileTest, PrintOneSample) {
  CdtFile file(TEST_DATA_PATH "noise.bin");
  file.read();
  for (const auto& r : file.Data) {
    MESSAGE() << r.debug() << "\n";
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
