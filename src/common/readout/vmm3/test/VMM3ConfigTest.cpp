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
};


TEST_F(VMM3ConfigTest, NotJson) {
  VMM3 testvmm3;
  testvmm3.FileName="/etc/passwd"; // not a jason file
  EXPECT_ANY_THROW(testvmm3.loadAndApplyConfig());
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
