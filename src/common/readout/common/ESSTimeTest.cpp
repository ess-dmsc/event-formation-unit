// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESSTime
///
//===----------------------------------------------------------------------===//

#include <readout/common/ESSTime.h>
#include <test/TestBase.h>
#include <cmath>

namespace ESSReadout {

class ESSTimeTest : public TestBase {
protected:
  ESSTime Time;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ESSTimeTest, Constructor) {
  ASSERT_EQ(Time.getTOF(0, 0), 0);
}

TEST_F(ESSTimeTest, SetRef) {
  Time.setReference(100, 0);
  ASSERT_EQ(Time.getTOF(100, 0),  0);
  ASSERT_EQ(Time.getTOF(100, 1), 11);
  ASSERT_EQ(Time.getTOF(100, 2), 22);
  ASSERT_EQ(Time.getTOF(100, 3), 34);
  ASSERT_EQ(Time.getTOF(200, 0), 100*1000000000LU);
}

TEST_F(ESSTimeTest, Bounds) {
  Time.setReference(0, 0);
  ASSERT_EQ(Time.getTOF(0, 88052499), 999999988);
  Time.setReference(0, 88052499);
  ASSERT_EQ(Time.getTOF(1, 0), 12); // why not 11?
}

TEST_F(ESSTimeTest, PrevPulse) {
  Time.setReference(100, 100000);
  Time.setPrevReference(100,  50000);

  ASSERT_EQ(Time.getTOF(100, 100000),  0);
  ASSERT_EQ(Time.getTOF(100, 75000), Time.InvalidTOF);

  ASSERT_EQ(Time.getPrevTOF(100, 75000), (uint64_t)(25000 * Time.NsPerTick));
  ASSERT_EQ(Time.getPrevTOF(100, 50000), 0);
  ASSERT_EQ(Time.getPrevTOF(100, 49999), Time.InvalidTOF);

  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofNegative, 1);
  ASSERT_EQ(Time.Stats.PrevTofCount, 2);
  ASSERT_EQ(Time.Stats.PrevTofNegative, 1);
}

TEST_F(ESSTimeTest, AddConstantDelay) {
  Time.setReference(0, 0);
  ASSERT_EQ(Time.getTOF(0, 88052499), 999999988);
  ASSERT_EQ(Time.getTOF(0, 88052499, 0), 999999988);
  ASSERT_EQ(Time.getTOF(0, 88052499, 11), 999999999);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
} // namespace ESSReadout
