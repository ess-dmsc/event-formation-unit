// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <common/time/ESSTime.h>
#include <cstdint>

using namespace esstime;

class ESSTimeTest : public TestBase {
protected:
  Statistics TestStats;
  ESSReferenceTime Time;

  ESSTimeTest() : Time(TestStats) {}

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ESSTimeTest, TestConversionHzToNanoseconds) {
  // Test with integer inputs
  auto period = esstime::hzToNanoseconds(1000);
  EXPECT_EQ(period.count(), 1000000); // 1000 Hz = 1ms = 1,000,000 ns

  period = esstime::hzToNanoseconds(1);
  EXPECT_EQ(period.count(), 1000000000); // 1 Hz = 1s = 1,000,000,000 ns

  period = esstime::hzToNanoseconds(10000);
  EXPECT_EQ(period.count(), 100000); // 10,000 Hz = 0.1ms = 100,000 ns

  // Test very high frequency (edge case)
  period = esstime::hzToNanoseconds(1000000000); // 1 GHz
  EXPECT_EQ(period.count(), 1);                  // 1 ns

  // Test with 14 Hz frequency
  period = esstime::hzToNanoseconds(14);
  EXPECT_EQ(period.count(), 71428571); // 14 Hz = 71,428,571 ns

  // Test edge cases
  EXPECT_THROW(esstime::hzToNanoseconds(0),
               std::invalid_argument); // Division by zero should throw
}

TEST_F(ESSTimeTest, TestConversionNsToMicroseconds) {
  // Test with TimeDurationNano input
  auto nanoDuration = esstime::TimeDurationNano(5000);
  auto microDuration = esstime::nsToMicroseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(microDuration.count(), 5.0); // 5000 ns = 5 µs

  // Test with integer input
  microDuration = esstime::nsToMicroseconds(5000);
  EXPECT_DOUBLE_EQ(microDuration.count(), 5.0); // 5000 ns = 5 µs

  microDuration = esstime::nsToMicroseconds(1);
  EXPECT_DOUBLE_EQ(microDuration.count(), 0.001); // 1 ns = 0.001 µs

  nanoDuration = esstime::TimeDurationNano(1);
  microDuration = esstime::nsToMicroseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(microDuration.count(), 0.001); // 1 ns = 0.001 µs

  // Test negative value with int64_t
  microDuration = esstime::nsToMicroseconds(-5000);
  EXPECT_DOUBLE_EQ(microDuration.count(), -5.0); // -5000 ns = -5 µs

  // Test negative value with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(-5000);
  microDuration = esstime::nsToMicroseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(microDuration.count(), -5.0); // -5000 ns = -5 µs

  // Test precision for fractional microseconds with int64_t
  microDuration = esstime::nsToMicroseconds(5123);
  EXPECT_DOUBLE_EQ(microDuration.count(), 5.123); // 5123 ns = 5.123 µs

  // Test precision for fractional microseconds with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(5123);
  microDuration = esstime::nsToMicroseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(microDuration.count(), 5.123); // 5123 ns = 5.123 µs

  // Test rounding behavior for fractional microseconds with int64_t
  microDuration = esstime::nsToMicroseconds(999);
  EXPECT_DOUBLE_EQ(microDuration.count(), 0.999); // 999 ns = 0.999 µs

  // Test rounding behavior for fractional microseconds with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(999);
  microDuration = esstime::nsToMicroseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(microDuration.count(), 0.999); // 999 ns = 0.999 µs
}

TEST_F(ESSTimeTest, TestConversionNsToMilliseconds) {
  // Test with integer input
  auto milliDuration = esstime::nsToMilliseconds(5000000);
  EXPECT_EQ(milliDuration.count(), 5.0); // 5,000,000 ns = 5 ms

  // Test with TimeDurationNano input
  esstime::TimeDurationNano nanoDuration(5000000);
  milliDuration = esstime::nsToMilliseconds(nanoDuration);
  EXPECT_EQ(milliDuration.count(), 5.0); // 5,000,000 ns = 5 ms

  // Test negative value with int64_t
  milliDuration = esstime::nsToMilliseconds(-5000000);
  EXPECT_EQ(milliDuration.count(), -5.0); // -5,000,000 ns = -5 ms

  // Test negative value with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(-5000000);
  milliDuration = esstime::nsToMilliseconds(nanoDuration);
  EXPECT_EQ(milliDuration.count(), -5.0); // -5,000,000 ns = -5 ms

  // Test precision for fractional milliseconds with int64_t
  milliDuration = esstime::nsToMilliseconds(5123456);
  EXPECT_DOUBLE_EQ(milliDuration.count(),
                   5.123456); // 5,123,456 ns = 5.123456 ms

  // Test precision for fractional milliseconds with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(5123456);
  milliDuration = esstime::nsToMilliseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(milliDuration.count(),
                   5.123456); // 5,123,456 ns = 5.123456 ms

  // Test boundary case with int64_t
  milliDuration = esstime::nsToMilliseconds(999999);
  EXPECT_DOUBLE_EQ(milliDuration.count(), 0.999999); // 999999 ns = 0.999999 ms

  // Test boundary case with TimeDurationNano
  nanoDuration = esstime::TimeDurationNano(999999);
  milliDuration = esstime::nsToMilliseconds(nanoDuration);
  EXPECT_DOUBLE_EQ(milliDuration.count(), 0.999999); // 999999 ns = 0.999999 ms
}

TEST_F(ESSTimeTest, TestConversionSToNanoseconds) {
  // Test with integer input
  auto nanoDuration = esstime::sToNanoseconds(5);
  EXPECT_EQ(nanoDuration.count(), 5000000000); // 5s = 5,000,000,000 ns

  // Test with TimeDurationSec input
  esstime::TimeDurationSec secDuration(5.0);
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(), 5000000000); // 5s = 5,000,000,000 ns

  // Test negative value with int64_t
  nanoDuration = esstime::sToNanoseconds(-5);
  EXPECT_EQ(nanoDuration.count(), -5000000000); // -5s = -5,000,000,000 ns

  // Test negative value with TimeDurationSec
  secDuration = esstime::TimeDurationSec(-5.0);
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(), -5000000000); // -5s = -5,000,000,000 ns

  // Test fractional second conversion with TimeDurationSec
  nanoDuration = esstime::sToNanoseconds(0.123456789);
  EXPECT_EQ(nanoDuration.count(), 123456789);

  // Test fractional second conversion with TimeDurationSec
  secDuration = esstime::TimeDurationSec(0.123456789);
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(), 123456789);

  // Test specific edge case for small fractional seconds with rounding concerns
  secDuration =
      esstime::TimeDurationSec(0.000000001); // 1 nanosecond as seconds
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(), 1); // Should be exactly 1 ns

  // Test rounding behavior with fractional nanoseconds
  secDuration = esstime::TimeDurationSec(0.0000000019); // 1.9 ns as seconds
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(), 1); // Should round to 2 ns

  // Test rounding behavior with fractional nanoseconds
  nanoDuration = esstime::sToNanoseconds(0.0000000019);
  EXPECT_EQ(nanoDuration.count(), 1); // Should round to 2 ns

  secDuration = esstime::TimeDurationSec(0.0000000004); // 0.4 ns as seconds
  nanoDuration = esstime::sToNanoseconds(secDuration);
  EXPECT_EQ(nanoDuration.count(),
            0); // Should round to 0 ns (truncation behavior)
}

TEST_F(ESSTimeTest, TestConversionSToMilliseconds) {
  // Test with integer input
  auto milliDuration = esstime::sToMilliseconds(5);
  EXPECT_EQ(milliDuration.count(), 5000.0); // 5s = 5,000 ms

  // Test with TimeDurationSec input
  esstime::TimeDurationSec secDuration(5.0);
  milliDuration = esstime::sToMilliseconds(secDuration);
  EXPECT_EQ(milliDuration.count(), 5000.0); // 5s = 5,000 ms

  // Test negative value with int64_t
  milliDuration = esstime::sToMilliseconds(-5);
  EXPECT_EQ(milliDuration.count(), -5000.0); // -5s = -5,000 ms

  // Test negative value with TimeDurationSec
  secDuration = esstime::TimeDurationSec(-5.0);
  milliDuration = esstime::sToMilliseconds(secDuration);
  EXPECT_EQ(milliDuration.count(), -5000.0); // -5s = -5,000 ms

  // Test precision for fractional seconds
  secDuration = esstime::TimeDurationSec(0.123456789);
  milliDuration = esstime::sToMilliseconds(secDuration);
  EXPECT_DOUBLE_EQ(milliDuration.count(),
                   123.456789); // 0.123456789s = 123.456789 ms

  // Test precision for fractional seconds
  milliDuration = esstime::sToMilliseconds(0.123456789);
  EXPECT_DOUBLE_EQ(milliDuration.count(),
                   123.456789); // 0.123456789s = 123.456789 ms

  // Test boundary case
  esstime::TimeDurationSec boundarySecDuration(0.0009999);
  milliDuration = esstime::sToMilliseconds(boundarySecDuration);
  EXPECT_DOUBLE_EQ(milliDuration.count(),
                   0.9999); // Should be exactly 0.9999 ms
}

TEST_F(ESSTimeTest, TestConversionMsToNanoseconds) {
  // Test with TimeDurationMilli input
  esstime::TimeDurationMilli milliDuration(5.0);
  auto nanoDuration = esstime::msToNanoseconds(milliDuration);
  EXPECT_EQ(nanoDuration.count(), 5000000); // 5ms = 5,000,000 ns

  // Test with double input (same value)
  nanoDuration = esstime::msToNanoseconds(5.0);
  EXPECT_EQ(nanoDuration.count(), 5000000); // 5ms = 5,000,000 ns

  // Test negative value with double
  nanoDuration = esstime::msToNanoseconds(-5.0);
  EXPECT_EQ(nanoDuration.count(), -5000000); // -5ms = -5,000,000 ns

  // Test negative value with TimeDurationMilli
  milliDuration = esstime::TimeDurationMilli(-5.0);
  nanoDuration = esstime::msToNanoseconds(milliDuration);
  EXPECT_EQ(nanoDuration.count(), -5000000); // -5ms = -5,000,000 ns

  // Test fractional value with double
  nanoDuration = esstime::msToNanoseconds(0.123);
  EXPECT_EQ(nanoDuration.count(), 123000); // 0.123ms = 123,000 ns

  // Test fractional value with TimeDurationMilli
  milliDuration = esstime::TimeDurationMilli(0.123);
  nanoDuration = esstime::msToNanoseconds(milliDuration);
  EXPECT_EQ(nanoDuration.count(), 123000); // 0.123ms = 123,000 ns

  // Test very small value with double (rounding behavior)
  nanoDuration = esstime::msToNanoseconds(0.0000009);
  EXPECT_EQ(nanoDuration.count(), 0); // 0.0000009ms ~ 0 due to duration cast

  // Test very small value with TimeDurationMilli (rounding behavior)
  milliDuration = esstime::TimeDurationMilli(0.0000009);
  nanoDuration = esstime::msToNanoseconds(milliDuration);
  EXPECT_EQ(nanoDuration.count(), 0); // 0.0000009ms ~ 0 due to duration cast

  // Test ceil rounding with double (below 0.5)
  nanoDuration = esstime::msToNanoseconds(0.0000004);
  EXPECT_EQ(nanoDuration.count(), 0); // 0.0000004ms ~ 0 ns due to duration cast

  // Test ceil rounding with TimeDurationMilli (below 0.5)
  milliDuration = esstime::TimeDurationMilli(0.0000004);
  nanoDuration = esstime::msToNanoseconds(milliDuration);
  EXPECT_EQ(nanoDuration.count(), 0); // 0.0000004ms ~ 0 ns due to duration cast
}

TEST_F(ESSTimeTest, ESSTimeArithmetic) {
  // Test adding nanoseconds
  esstime::ESSTime time(0, 0);
  time +=
      esstime::TimeDurationNano(1000000000); // Add 1 second = 1,000,000,000 ns
  EXPECT_EQ(time.getTimeHigh(), 1);
  EXPECT_EQ(time.getTimeLow(), 0);

  // Test addition with large nanoseconds value
  esstime::ESSTime time2(1, 0);
  auto result = time2 + esstime::TimeDurationNano(2000000000); // Add 2 seconds
  EXPECT_EQ(result.getTimeHigh(), 3);
  EXPECT_EQ(result.getTimeLow(), 0);

  // Test subtraction between two ESSTime objects
  esstime::ESSTime time3(5, 0);
  esstime::ESSTime time4(3, 0);
  auto diff = time3 - time4;
  EXPECT_EQ(diff.count(), 2000000000); // 2 seconds = 2,000,000,000 ns

  // Test addition with ticks
  esstime::ESSTime time5(1, 0);
  time5 += 44026250; // Add half a second of ticks (88,052,500 / 2)
  EXPECT_EQ(time5.getTimeHigh(), 1);
  EXPECT_EQ(time5.getTimeLow(), 44026250);
}

TEST_F(ESSTimeTest, ESSTimeEdgeCases) {
  // Test wrapping around with ticks
  esstime::ESSTime time(0, 88052499); // One tick below wrap point
  time += 2;                          // Should wrap to next second
  EXPECT_EQ(time.getTimeHigh(), 1);
  EXPECT_EQ(time.getTimeLow(), 1);

  // Test adding large number of ticks
  esstime::ESSTime time2(0, 0);
  time2 += 176105000; // 2 seconds worth of ticks
  EXPECT_EQ(time2.getTimeHigh(), 2);
  EXPECT_EQ(time2.getTimeLow(), 0);
}

//---------------------------------------------------------------------
// Test ESSReferenceTime class
//---------------------------------------------------------------------

TEST_F(ESSTimeTest, Constructors) {
  Statistics testStats1, testStats2;
  ESSReferenceTime testTime1 = ESSReferenceTime(testStats1);
  ASSERT_EQ(testTime1.getTOF(ESSTime(0, 0)), 0);
}

TEST_F(ESSTimeTest, SetRef) {
  Time.setReference(ESSTime(100, 0));
  ASSERT_EQ(Time.getTOF(ESSTime(100, 0)), 0);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 1)), 11);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 2)), 22);
  ASSERT_EQ(Time.getTOF(ESSTime(100, 3)), 34);
  ASSERT_EQ(Time.getTOF(ESSTime(200, 0)), Time.InvalidTOF);

  ASSERT_EQ(Time.Counters.TofCount, 4);
  ASSERT_EQ(Time.Counters.TofNegative, 0);
  ASSERT_EQ(Time.Counters.TofHigh, 1);
  ASSERT_EQ(Time.Counters.PrevTofCount, 0);
  ASSERT_EQ(Time.Counters.PrevTofNegative, 0);
  ASSERT_EQ(Time.Counters.PrevTofHigh, 0);
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

  ASSERT_EQ(Time.Counters.TofCount, 1);
  ASSERT_EQ(Time.Counters.TofNegative, 2);
  ASSERT_EQ(Time.Counters.TofHigh, 0);
  ASSERT_EQ(Time.Counters.PrevTofCount, 3);
  ASSERT_EQ(Time.Counters.PrevTofNegative, 2);
  ASSERT_EQ(Time.Counters.PrevTofHigh, 0);
}

TEST_F(ESSTimeTest, AddConstantDelay) {
  Time.setReference(ESSTime(0, 0));
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499)), 999999988);
  ASSERT_EQ(Time.Counters.TofCount, 1);
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499), 0), 999999988);
  ASSERT_EQ(Time.Counters.TofCount, 2);
  ASSERT_EQ(Time.getTOF(ESSTime(0, 88052499), 11), 999999999);
  ASSERT_EQ(Time.Counters.TofCount, 3);
}

TEST_F(ESSTimeTest, MaxTOFExceeded) {
  Time.setReference(ESSTime(0, 0));

  Time.getTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Counters.TofCount, 1);
  ASSERT_EQ(Time.Counters.TofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Counters.TofCount, 1);
  ASSERT_EQ(Time.Counters.TofHigh, 1);
}

TEST_F(ESSTimeTest, MaxPrevTOFExceeded) {
  Time.setPrevReference(ESSTime(0, 0));

  Time.getPrevTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Counters.PrevTofCount, 1);
  ASSERT_EQ(Time.Counters.PrevTofHigh, 0);

  Time.setMaxTOF(0x01);
  Time.getPrevTOF(ESSTime(0, 2));
  ASSERT_EQ(Time.Counters.PrevTofCount, 1);
  ASSERT_EQ(Time.Counters.PrevTofHigh, 1);
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
