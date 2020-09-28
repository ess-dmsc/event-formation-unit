// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for BuilderVMM3
///
/// The tests is a type of module integration test that requires several other
/// classes and uses test data from ParserVMM3TestData.h
//===----------------------------------------------------------------------===//

#include <gdgem/NMXStats.h>
#include <gdgem/srs/BuilderVMM3.h>
#include <gdgem/srs/ParserVMM3TestData.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <test/TestBase.h>

using namespace Gem;

class BuilderVMM3Test : public TestBase {

protected:
  BuilderVMM3 * builder;
  uint16_t AdcThreshold{0};
  std::string DumpDir{""};
  NMXStats stats;
  SRSTime srsTime;
  SRSMappings srsMappings;
  std::shared_ptr<CalibrationFile> calibration;
  void SetUp() override {
    calibration = std::make_shared<CalibrationFile>();

    builder = new BuilderVMM3(srsTime, srsMappings,
      AdcThreshold, DumpDir,
      0, 1279, 1280,
      calibration, stats, true);
  }


};

/** Test cases below */

// Data packet captured from wireshark some time ago gotten from
// GdGemBaseTestData.h
TEST_F(BuilderVMM3Test, ProcessData) {
    builder->process_buffer((char * )marker_3_data_3.data(), marker_3_data_3.size());
    ASSERT_EQ(stats.ParserBadFrames, 0);
    ASSERT_EQ(stats.ParserReadouts, 6);
    ASSERT_EQ(stats.ParserData, 3);
    ASSERT_EQ(stats.ParserMarkers, 3);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
