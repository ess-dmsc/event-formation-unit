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
#include <common/testutils/TestBase.h>
#include <common/testutils/SaveBuffer.h>

//
std::vector<uint8_t> marker_3_data_3_plane_0_and_1 {
  0x00, 0x33, 0x71, 0x37,             // Frame Counter
  0x56, 0x4d, 0x33, 0x10,             // Data ID 564d33 and fecId 1
  0x4c, 0x39, 0x2f, 0x60,             // UDP timestamp
  0x00, 0x00, 0x00, 0x00,             // Offset overflow last frame
  0x00, 0x00, 0x00, 0x00, 0x04, 0x01, // marker 1: vmm1, timeStamp 1
  0x00, 0x00, 0x00, 0x00, 0x08, 0x02, // marker 2: vmm2, timeStamp 2
  0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, // marker 3: vmm3, timeStamp 3
  0xe0, 0x92, 0x24, 0x02, 0x81, 0x00, // hit 1, vmm 2, channel 1, adc 290
  0xe0, 0x92, 0x34, 0x01, 0x82, 0x00, // hit 2  vmm 2, channel 2, adc 291
  0xe0, 0xd2, 0x20, 0x22, 0x83, 0x00  // hit 3  vmm 3, channel 3, adc 290
};

using namespace Gem;

class BuilderVMM3Test : public TestBase {

protected:
  const int fec1{1};
  const int vmm2{2};
  const int vmm3{3};
  const int plane0{0};
  const int plane1{1};
  const uint16_t AdcThreshold0{0};
  const uint16_t AdcThreshold291{291};

  BuilderVMM3 * builder;

  std::string DumpDir{""};
  NMXStats stats;
  SRSTime srsTime;
  SRSMappings srsMappings;
  std::shared_ptr<CalibrationFile> calibration;

  void SetUp() override {
    calibration = std::make_shared<CalibrationFile>();

    builder = new BuilderVMM3(srsTime, srsMappings,
      AdcThreshold291, DumpDir,
      0, 1279, 1280,
      calibration, stats, true);
  }
};

//
//  Test cases below
//

// Data packet captured from wireshark some time ago gotten from
// ParserVMM3TestData.h
TEST_F(BuilderVMM3Test, ProcessDataBadMapping) {
    builder->process_buffer((char * )marker_3_data_3.data(), marker_3_data_3.size());
    ASSERT_EQ(stats.ParserBadFrames, 0);
    ASSERT_EQ(stats.ParserReadouts, 6);
    ASSERT_EQ(stats.ParserData, 3);
    ASSERT_EQ(stats.ParserMarkers, 3);
    ASSERT_EQ(stats.HitsBadPlane, 3);
}


// Data packet captured from wireshark some time ago gotten from
// ParserVMM3TestData.h
TEST_F(BuilderVMM3Test, ProcessDataGoodMapping) {
    srsMappings.set_mapping(fec1, vmm2, plane0, 0); // FEC 1, VMM 2, plane 0, offset 0
    BuilderVMM3 * builderGood = new BuilderVMM3(srsTime, srsMappings,
    AdcThreshold291, "deleteme_", 0, 1279, 1280, calibration, stats, true);

    builderGood->process_buffer((char * )marker_3_data_3.data(), marker_3_data_3.size());
    ASSERT_EQ(stats.HitsBadPlane, 0);
    ASSERT_EQ(stats.HitsBadAdc, 2); // see threshold value above
    ASSERT_EQ(stats.HitsOutsideRegion, 0);
    deleteFile(builderGood->getFilename());
}

TEST_F(BuilderVMM3Test, ProcessDataBothPlanes) {
    srsMappings.set_mapping(fec1, vmm2, plane0, 0); // FEC 1, VMM 2, plane 0, offset 0
    srsMappings.set_mapping(fec1, vmm3, plane1, 0); // FEC 1, VMM 2, plane 0, offset 0
    BuilderVMM3 * builderGood = new BuilderVMM3(srsTime, srsMappings,
    AdcThreshold0, DumpDir, 0, 1279, 1280, calibration, stats, true);

    builderGood->process_buffer((char * )marker_3_data_3_plane_0_and_1.data(),
      marker_3_data_3_plane_0_and_1.size());
    ASSERT_EQ(stats.HitsBadPlane, 0);
    ASSERT_EQ(stats.HitsBadAdc, 0); // see threshold value above

}

TEST_F(BuilderVMM3Test, ProcessHitOutsideRegion) {
    srsMappings.set_mapping(fec1, vmm2, plane0, 0); // FEC 1, VMM 2, plane 0, offset 0
    srsMappings.set_mapping(fec1, vmm3, plane1, 0); // FEC 1, VMM 2, plane 0, offset 0
    BuilderVMM3 * builderGood = new BuilderVMM3(srsTime, srsMappings,
    AdcThreshold0, DumpDir, 800, 1000, 100, calibration, stats, true);

    builderGood->process_buffer((char * )marker_3_data_3_plane_0_and_1.data(),
      marker_3_data_3_plane_0_and_1.size());
    ASSERT_EQ(stats.HitsBadPlane, 0);
    ASSERT_EQ(stats.HitsBadAdc, 0); // see threshold value above
    ASSERT_EQ(stats.HitsOutsideRegion, 2);
}

// Seems like the bad geometry is practically unreachable due
// to previous checks
// TEST_F(BuilderVMM3Test, ProcessDataBadGeometry) {
//     ASSERT_EQ(stats.HitsBadPlane, 0);
//     srsMappings.set_mapping(1,2,0,0); // FEC 1, VMM 2, plane 0, offset 0
//     BuilderVMM3 * builderGood = new BuilderVMM3(srsTime, srsMappings,
//     AdcThreshold, DumpDir,
//     0, 1279, 1280,
//     calibration, stats, true);
//     builderGood->process_buffer((char * )marker_3_data_3_bad_vmmid.data(), marker_3_data_3_bad_vmmid.size());
//     ASSERT_EQ(stats.HitsBadGeometry, 3); // vmm 3 is not defined
// }



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
