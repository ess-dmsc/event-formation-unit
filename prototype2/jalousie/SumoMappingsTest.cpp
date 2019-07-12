/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/SumoMappings.h>
#include <test/TestBase.h>
#include <jalousie/rapidcsv.h>

using namespace Jalousie;

class JalSumoMappingsTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(JalSumoMappingsTest, TestFile) {
  rapidcsv::Document doc(TEST_DATA_PATH "sumo_voxel_map_20190711.csv",
                         rapidcsv::LabelParams(0, -1),
                         rapidcsv::SeparatorParams(';'));

  std::vector<int> line;
  line = doc.GetRow<int>(0);
  EXPECT_EQ(line, std::vector<int>({3, 0, 0, 0, 0, 0}));
  line = doc.GetRow<int>(1);
  EXPECT_EQ(line, std::vector<int>({3, 0, 0, 1, 0, 1}));
  line = doc.GetRow<int>(31);
  EXPECT_EQ(line, std::vector<int>({3, 0, 1, 15, 1, 15}));

  EXPECT_EQ(doc.GetRowCount(), 14336);
  line = doc.GetRow<int>(14335);
  EXPECT_EQ(line, std::vector<int>({6, 19, 15, 15, 63, 127}));

  for (size_t i=0; i < doc.GetRowCount(); ++i) {
    line = doc.GetRow<int>(i);
    assert(line[0] >= 0);
    assert(line[1] >= 0);
    assert(line[2] >= 0);
    assert(line[3] >= 0);
    assert(line[4] >= 0);
    assert(line[5] >= 0);

    assert(line[0] <= 254);
    assert(line[1] <= 254);
    assert(line[2] <= 254);
    assert(line[3] <= 254);
    assert(line[4] <= 254);
    assert(line[5] <= 254);
  }
}

TEST_F(JalSumoMappingsTest, TestParsing3) {
  SumoMappings mappings(TEST_DATA_PATH "sumo_voxel_map_20190711.csv", 3);
  MESSAGE() << "\n" << mappings.debug(false) << "\n";
}

TEST_F(JalSumoMappingsTest, TestParsing4) {
  SumoMappings mappings(TEST_DATA_PATH "sumo_voxel_map_20190711.csv", 4);
  MESSAGE() << "\n" << mappings.debug(false) << "\n";
}

TEST_F(JalSumoMappingsTest, TestParsing5) {
  SumoMappings mappings(TEST_DATA_PATH "sumo_voxel_map_20190711.csv", 5);
  MESSAGE() << "\n" << mappings.debug(false) << "\n";
}

TEST_F(JalSumoMappingsTest, TestParsing6) {
  SumoMappings mappings(TEST_DATA_PATH "sumo_voxel_map_20190711.csv", 6);
  MESSAGE() << "\n" << mappings.debug(false) << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
