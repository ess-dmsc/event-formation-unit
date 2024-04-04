// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESSTime
///
//===----------------------------------------------------------------------===//

#include <cmath>
#include <common/readout/ess/ESSTime.h>
#include <common/testutils/TestBase.h>

namespace ESSReadout {

class ESSTimeTest : public TestBase {
protected:
  ESSTime Time;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ESSTimeTest, Constructor) { ASSERT_EQ(Time.getTOF(0, 0), 0); }

TEST_F(ESSTimeTest, SetRef) {
  Time.setReference(100, 0);
  ASSERT_EQ(Time.getTOF(100, 0), 0);
  ASSERT_EQ(Time.getTOF(100, 1), 11);
  ASSERT_EQ(Time.getTOF(100, 2), 22);
  ASSERT_EQ(Time.getTOF(100, 3), 34);
  ASSERT_EQ(Time.getTOF(200, 0), Time.InvalidTOF);

  ASSERT_EQ(Time.Stats.TofCount, 4);
  ASSERT_EQ(Time.Stats.TofNegative, 0);
  ASSERT_EQ(Time.Stats.TofHigh, 1);
  ASSERT_EQ(Time.Stats.PrevTofCount, 0);
  ASSERT_EQ(Time.Stats.PrevTofNegative, 0);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);
}

TEST_F(ESSTimeTest, Bounds) {
  Time.setReference(0, 0);
  ASSERT_EQ(Time.getTOF(0, 88052499), 999999988);
  Time.setReference(0, 88052499);
  ASSERT_EQ(Time.getTOF(1, 0), 12); // why not 11?
}

TEST_F(ESSTimeTest, PrevPulse) {
  Time.setReference(100, 100000);
  Time.setPrevReference(100, 50000);

  ASSERT_EQ(Time.getTOF(100, 100000), 0);
  ASSERT_EQ(Time.getTOF(100, 75000), (uint64_t)(25000 * Time.NsPerTick));
  ASSERT_EQ(Time.getTOF(100, 49999), Time.InvalidTOF);

  ASSERT_EQ(Time.getPrevTOF(100, 75000), (uint64_t)(25000 * Time.NsPerTick));
  ASSERT_EQ(Time.getPrevTOF(100, 50000), 0);
  ASSERT_EQ(Time.getPrevTOF(100, 49999), Time.InvalidTOF);

  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofNegative, 2);
  ASSERT_EQ(Time.Stats.TofHigh, 0);
  ASSERT_EQ(Time.Stats.PrevTofCount, 3);
  ASSERT_EQ(Time.Stats.PrevTofNegative, 2);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);
}

TEST_F(ESSTimeTest, AddConstantDelay) {
  Time.setReference(0, 0);
  ASSERT_EQ(Time.getTOF(0, 88052499), 999999988);
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.getTOF(0, 88052499, 0), 999999988);
  ASSERT_EQ(Time.Stats.TofCount, 2);
  ASSERT_EQ(Time.getTOF(0, 88052499, 11), 999999999);
  ASSERT_EQ(Time.Stats.TofCount, 3);
}

TEST_F(ESSTimeTest, MaxTOFExceeded) {
  Time.setReference(0, 0);

  Time.getTOF(0, 2);
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getTOF(0, 2);
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofHigh, 1);
}

TEST_F(ESSTimeTest, MaxPrevTOFExceeded) {
  Time.setPrevReference(0, 0);

  Time.getPrevTOF(0, 2);
  ASSERT_EQ(Time.Stats.PrevTofCount, 1);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getPrevTOF(0, 2);
  ASSERT_EQ(Time.Stats.PrevTofCount, 1);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 1);
}

TEST_F(ESSTimeTest, ExactPulseDelay) {
  Time.setReference(0, 6289464);
  ASSERT_EQ(Time.TimeInNS, 71428568);

  Time.setReference(0, 0x034a3c11);
  Time.setPrevReference(0, 0x02ea43d9);
  ASSERT_EQ(Time.TimeInNS - Time.PrevTimeInNS, 71428568);

  Time.setReference(0x615c414f, 0x034a3c11);
  Time.setPrevReference(0x615c414f, 0x02ea43d9);
  ASSERT_EQ(Time.TimeInNS - Time.PrevTimeInNS, 71428568);
}

// Test specific error condition
TEST_F(ESSTimeTest, Issue2021_11_08) {
  Time.setReference(0x6188e7a9, 0x00070ff6);
  ASSERT_EQ(Time.TimeInNS, 1636362153005256386);

  Time.setPrevReference(0x6188e7a8, 0x04e6aad1);
  ASSERT_EQ(Time.PrevTimeInNS, 1636362152933827807);
  ASSERT_EQ(Time.TimeInNS - Time.PrevTimeInNS, 71428579);
}

// Test time conversion back to 32bit high and low
TEST_F(ESSTimeTest, TestPulseTimeConversion) {
  uint32_t High = 0x6188e7a9;
  uint32_t Low = 0x00070ff6;
  Time.setReference(High, Low);

  ASSERT_EQ(Time.TimeInNS, 1636362153005256386);

  ESSTime::PulseTime pulseTime = Time.getPulseTime();
  ASSERT_EQ(pulseTime.TimeHigh, High);
  ASSERT_EQ(pulseTime.TimeLow, Low);
}

} // namespace ESSReadout

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
