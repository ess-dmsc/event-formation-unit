// Copyright (C) 2019 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <common/time/ESSTime.h>
#include <cstdint>

using namespace esstime;

class ESSTimeTest : public TestBase {
protected:
  ESSReferenceTime Time;
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
// Test ESSReferenceTime class
//---------------------------------------------------------------------

TEST_F(ESSTimeTest, Constructors) {
  ESSReferenceTime testTime1 = ESSReferenceTime();
  ESSReferenceTime testTime2 = ESSReferenceTime(ESSTime(0, 1));
  ASSERT_EQ(testTime1.getTOF(ESSTime(0, 0)), 0);
  ASSERT_EQ(testTime2.getTOF(ESSTime(0, 2)), 11);
  }


TEST_F(ESSTimeTest, SetRef) {
  Time.setReference(ESSTime(100, 0));
  ASSERT_EQ(Time.getTOF(ESSTime(100, 0)), 0);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 1)), 11);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 2)), 22);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 3)), 34);
  ASSERT_EQ(Time.getTOF(ESSTime(200, 0)), Time.InvalidTOF);

  ASSERT_EQ(Time.Stats.TofCount, 4);
  ASSERT_EQ(Time.Stats.TofNegative, 0);
  ASSERT_EQ(Time.Stats.TofHigh, 1);
  ASSERT_EQ(Time.Stats.PrevTofCount, 0);
  ASSERT_EQ(Time.Stats.PrevTofNegative, 0);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);
}

TEST_F(ESSTimeTest, Bounds) {
  Time.setReference(ESSTime(0, 0));
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499)), 999999988);
  Time.setReference(ESSTime(0, 88052499));
  ASSERT_EQ(Time.getTOF(ESSTime(1, 0)), 12); // why not 11?
}

TEST_F(ESSTimeTest, PrevPulse) {
  Time.setReference(ESSTime(100, 100000));
  Time.setPrevReference(ESSTime(100, 50000));

  ASSERT_EQ(Time.getTOF(ESSTime(100, 100000)), 0);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 75000)),
            (uint64_t)(25000 * ESSTime::ESSClockTick));
  ASSERT_EQ(Time.getTOF(ESSTime(100, 49999)), Time.InvalidTOF);

  ASSERT_EQ(Time.getPrevTOF(ESSTime(100, 75000)),
            (uint64_t)(25000 * ESSTime::ESSClockTick));
  ASSERT_EQ(Time.getPrevTOF(ESSTime(100, 50000)), 0);
  ASSERT_EQ(Time.getPrevTOF(ESSTime(100, 49999)), Time.InvalidTOF);

  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofNegative, 2);
  ASSERT_EQ(Time.Stats.TofHigh, 0);
  ASSERT_EQ(Time.Stats.PrevTofCount, 3);
  ASSERT_EQ(Time.Stats.PrevTofNegative, 2);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);
}

TEST_F(ESSTimeTest, AddConstantDelay) {
  Time.setReference(ESSTime(0, 0));
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499)), 999999988);
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499), 0), 999999988);
  ASSERT_EQ(Time.Stats.TofCount, 2);
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499), 11), 999999999);
  ASSERT_EQ(Time.Stats.TofCount, 3);
}

TEST_F(ESSTimeTest, MaxTOFExceeded) {
  Time.setReference(ESSTime(0, 0));

  Time.getTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Stats.TofCount, 1);
  ASSERT_EQ(Time.Stats.TofHigh, 1);
}

TEST_F(ESSTimeTest, MaxPrevTOFExceeded) {
  Time.setPrevReference(ESSTime(0, 0));

  Time.getPrevTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Stats.PrevTofCount, 1);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getPrevTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Stats.PrevTofCount, 1);
  ASSERT_EQ(Time.Stats.PrevTofHigh, 1);
}

TEST_F(ESSTimeTest, ExactPulseDelay) {
  Time.setReference(ESSTime(0, 6289464));
  ASSERT_EQ(Time.getRefTimeUInt64(), 71428568);

  Time.setReference(ESSTime(0, 0x034a3c11));
  Time.setPrevReference(ESSTime(0, 0x02ea43d9));
  ASSERT_EQ(Time.getRefTimeUInt64() - Time.getPrevRefTimeUInt64(), 71428568);

  Time.setReference(ESSTime(0x615c414f, 0x034a3c11));
  Time.setPrevReference(ESSTime(0x615c414f, 0x02ea43d9));
  ASSERT_EQ(Time.getRefTimeUInt64() - Time.getPrevRefTimeUInt64(), 71428568);
}

// Test specific error condition
TEST_F(ESSTimeTest, Issue2021_11_08) {
  Time.setReference(ESSTime(0x6188e7a9, 0x00070ff6));
  ASSERT_EQ(Time.getRefTimeUInt64(), 1636362153005256386);

  Time.setPrevReference(ESSTime(0x6188e7a8, 0x04e6aad1));
  ASSERT_EQ(Time.getPrevRefTimeUInt64(), 1636362152933827807);
  ASSERT_EQ(Time.getRefTimeUInt64() - Time.getPrevRefTimeUInt64(), 71428579);
}

// Test time conversion back to 32bit high and low
TEST_F(ESSTimeTest, RefTimeUnitConversion) {
  uint32_t High = 0x6188e7a9;
  uint32_t Low = 0x00070ff6;
  uint64_t timeNs = 1636362153005256386;
  Time.setReference(ESSTime(High, Low));
  Time.setPrevReference(ESSTime(High, Low));

  ASSERT_EQ(Time.getRefTimeUInt64(), timeNs);
  ASSERT_EQ(Time.getPrevRefTimeUInt64(), timeNs);
  ASSERT_EQ(Time.getRefTimeNS(), TimeDurationNano(timeNs));
  ASSERT_EQ(Time.getPrevRefTimeNS(), TimeDurationNano(timeNs));

  ESSTime pulseTime = Time.getRefESSTime();
  ASSERT_EQ(pulseTime.getTimeHigh(), High);
  ASSERT_EQ(pulseTime.getTimeLow(), Low);
  ESSTime prevPulseTime = Time.getPrevRefESSTime();
  ASSERT_EQ(prevPulseTime.getTimeHigh(), High);
  ASSERT_EQ(prevPulseTime.getTimeLow(), Low);
}

//---------------------------------------------------------------------
// Test ESSTime class
//---------------------------------------------------------------------

TEST_F(ESSTimeTest, ESSTimeIncreasedNumberOfTicks) {
  uint32_t high = 1000000000;
  uint32_t increase1000 = 1000;
  uint32_t low = 0;
  uint32_t multipleFreqBin = ESSTime::ESSClockFreqHz * 3 + increase1000;

  // We can increase with a value less than the frequency
  ESSTime testTime = ESSTime(high, low);
  testTime += increase1000;
  EXPECT_EQ(testTime.getTimeHigh(), high);
  EXPECT_EQ(testTime.getTimeLow(), increase1000);

  // EDGE TEST: test increase with the frequency should overflow
  testTime = ESSTime(high, ESSTime::ESSClockFreqHz - 100);
  testTime += 110;
  EXPECT_EQ(testTime.getTimeHigh(), high + 1);
  EXPECT_EQ(testTime.getTimeLow(), 10);

  // EDGE TEST: test increase with the frequency should overflow
  testTime = ESSTime(high, low);
  testTime += static_cast<uint32_t>(ESSTime::ESSClockFreqHz);
  EXPECT_EQ(testTime.getTimeHigh(), high + 1);
  EXPECT_EQ(testTime.getTimeLow(), 0);

  // We can increase with a multiple of the frequency
  testTime = ESSTime(high, low);
  testTime += multipleFreqBin;
  EXPECT_EQ(testTime.getTimeHigh(), high + 3);
  EXPECT_EQ(testTime.getTimeLow(), 1000);
}

TEST_F(ESSTimeTest, ESSTimeIncreasedNanosecDuration) {
  uint32_t high = 1000000000;
  uint32_t low = 0;

  // We can increase with 1000 nonsec which will increase only time low
  // with uint(1000 / clock tick)
  ESSTime testTime = ESSTime(high, low);
  testTime += TimeDurationNano(1000);
  EXPECT_EQ(testTime.getTimeHigh(), high);
  EXPECT_EQ(testTime.getTimeLow(),
            static_cast<uint32_t>(1000 / ESSTime::ESSClockTick));

  // EDGE TEST: We can increase with 1s and time low should overflow
  testTime = ESSTime(high, low); // reset
  testTime += TimeDurationNano(ESSTime::SecInNs);
  EXPECT_EQ(testTime.getTimeHigh(), high + 1);
  EXPECT_EQ(testTime.getTimeLow(), 0);

  // EDGE TEST: We can increase with frequency multiplyed by the clock tick and
  // overflow
  testTime = ESSTime(high, low); // reset
  testTime += TimeDurationNano(
      static_cast<uint64_t>(ESSTime::ESSClockFreqHz * ESSTime::ESSClockTick));
  EXPECT_EQ(testTime.getTimeHigh(), high + 1);
  EXPECT_EQ(testTime.getTimeLow(), 0);

  // We can increase with frequency multiplied by the clock tick plus the
  // tick/ns = 12ns plus the clock tick and it will overflow
  testTime = ESSTime(high, low); // reset
  testTime += TimeDurationNano(
      static_cast<uint64_t>(ESSTime::ESSClockFreqHz * ESSTime::ESSClockTick) +
      12);
  EXPECT_EQ(testTime.getTimeHigh(), high + 1);
  EXPECT_EQ(testTime.getTimeLow(), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
