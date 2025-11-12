// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for VMM3Config
///
//===----------------------------------------------------------------------===//

#include <common/readout/vmm3/VMM3Config.h>
#include <common/testutils/TestBase.h>

#include <filesystem>

using namespace vmm3;

using std::filesystem::path;

class VMM3 : public VMM3Config {
  void applyConfig() {}
};

class VMM3ConfigTest : public TestBase {
protected:
  VMM3 testvmm3;

  static void SetUpTestCase() {
    // Get base test dir
    TestDir = path(__FILE__).parent_path();

    // Define test files
    VMMConfigFile      = TestDir / path("vmm_config.json");
    VMMInvalidJSONFile = TestDir / path("invalid_json_file.json");
  }

  inline static path TestDir{""};
  inline static path VMMConfigFile{""};
  inline static path VMMInvalidJSONFile{""};
};


TEST_F(VMM3ConfigTest, ConfigFileNotJson) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.setConfigFile(VMMInvalidJSONFile.string());
  EXPECT_ANY_THROW(testvmm3.loadAndApplyConfig());
}

TEST_F(VMM3ConfigTest, ConfigFileDuplicateHybrid) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.setRootFromFile(VMMConfigFile.string());

  testvmm3["Config"][3]["HybridId"] = testvmm3["Config"][0]["HybridId"];
  EXPECT_ANY_THROW(testvmm3.applyVMM3Config());
}

TEST_F(VMM3ConfigTest, InvalidHybridId) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.setRootFromFile(VMMConfigFile.string());
  testvmm3.applyVMM3Config();

  EXPECT_FALSE(testvmm3.lookupHybrid("I do not exist"));
  EXPECT_ANY_THROW(testvmm3.getHybrid("I do not exist"));
}

TEST_F(VMM3ConfigTest, ValidHybridId) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.setRootFromFile(VMMConfigFile.string());
  testvmm3.applyVMM3Config();

  const std::string Id = "a0800006b882a0803410082006704410";
  ASSERT_TRUE(testvmm3.lookupHybrid(Id));

  const auto Hybrid = testvmm3.getHybrid(Id);
  EXPECT_EQ(Hybrid.HybridId, Id);
}

TEST_F(VMM3ConfigTest, ConfigPerformance) {
  testvmm3.ExpectedName = "Freia";
  testvmm3.setRootFromFile(VMMConfigFile.string());
  testvmm3.applyVMM3Config();

  // Get all Id's and test they are correct
  for (const auto & [Id, Hybrid]: testvmm3.HybridMap) {
    EXPECT_EQ(Id, Hybrid->HybridId);
  }
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
