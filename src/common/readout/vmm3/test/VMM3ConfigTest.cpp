// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for VMM3Config
///
//===----------------------------------------------------------------------===//

#include <common/readout/vmm3/VMM3Config.h>
#include <common/testutils/TestBase.h>



class VMM3 : public VMM3Config {
  void applyConfig() {}
};

class VMM3ConfigTest : public TestBase {
protected:
  VMM3 testvmm3;
};


TEST_F(VMM3ConfigTest, ConfigFileNotJson) {
  testvmm3.FileName="/etc/passwd"; // not a jason file
  EXPECT_ANY_THROW(testvmm3.loadAndApplyConfig());
}


TEST_F(VMM3ConfigTest, CalibFileNotJson) {
  EXPECT_ANY_THROW(testvmm3.loadAndApplyCalibration("/etc/passwd"));
}


TEST_F(VMM3ConfigTest, ConfigFileDuplicateHybrid) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.root = from_json_file(VMM_CONFIG_FILE);

  testvmm3.root["Config"][3]["HybridId"] = testvmm3.root["Config"][0]["HybridId"];
  EXPECT_ANY_THROW(testvmm3.applyVMM3Config());
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
