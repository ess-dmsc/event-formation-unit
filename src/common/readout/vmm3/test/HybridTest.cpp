// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Hybrid class
///
//===----------------------------------------------------------------------===//

#include <common/readout/vmm3/Hybrid.h>
#include <common/testutils/TestBase.h>

using namespace vmm3;

class HybridTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HybridTest, IsAvailable) {
  std::vector<Hybrid> Hybrids;
  Hybrid tmp;
  tmp.HybridId = "AAA";
  Hybrids.push_back(tmp);
  Hybrids.push_back(tmp);
  ASSERT_EQ(Hybrids.size(), 2);

  bool res = Hybrid::isAvailable("CCC", Hybrids);
  ASSERT_TRUE(res);

  res = Hybrid::isAvailable("AAA", Hybrids);
  ASSERT_FALSE(res);
}

TEST_F(HybridTest, ADCThresholds) {
  std::vector<Hybrid> Hybrids;
  Hybrid tmp;
  EXPECT_EQ(tmp.ADCThresholds.size(), 2);
  EXPECT_EQ(tmp.ADCThresholds[0].size(), 0);
  EXPECT_EQ(tmp.ADCThresholds[1].size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
