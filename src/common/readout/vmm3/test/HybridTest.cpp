// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Hybrid class
///
//===----------------------------------------------------------------------===//

#include <common/readout/vmm3/Hybrid.h>
#include <common/testutils/TestBase.h>

namespace ESSReadout {

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

  bool res = ESSReadout::Hybrid::isAvailable("CCC", Hybrids);
  ASSERT_TRUE(res);

  res = ESSReadout::Hybrid::isAvailable("AAA", Hybrids);
  ASSERT_FALSE(res);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
} // namespace ESSReadout
