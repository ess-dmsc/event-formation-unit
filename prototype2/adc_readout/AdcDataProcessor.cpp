/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Utility code for processing ADC data.
 */

#include "AdcDataProcessor.h"

AdcDataProcessor::AdcDataProcessor(std::shared_ptr<Producer> Prod) : ProducerPtr(Prod) {
  
}

namespace TimeStamp {
  static const std::uint32_t TimerCounterMax = 88052500/2;
  //static const std::uint32_t TimerCounterMax = 43478000;
  
  std::uint64_t Calc(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac) {
    std::uint64_t NanoSec = static_cast<std::uint64_t>((static_cast<double>(SecondsFrac) / static_cast<double>(TimerCounterMax)) * 1e9 + 0.5);
    return static_cast<std::uint64_t>(static_cast<std::uint64_t>(Seconds) * 1000000000 + NanoSec);
  }
  
  //Note: This function might be significantly slower than CalcTimeStamp() for some cases.
  std::uint64_t CalcFast(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac) {
    const std::uint64_t Multiplier = 100000000000;
    std::uint64_t NanoSec = (((SecondsFrac * Multiplier) / (TimerCounterMax)) + 50) / 100;
    return static_cast<std::uint64_t>(static_cast<std::uint64_t>(Seconds) * 1000000000 + NanoSec);
  }
  
  std::uint64_t CalcSample(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac, const std::uint32_t &SampleNr) {
    if (SecondsFrac + SampleNr >= TimerCounterMax) {
      return TimeStamp::Calc(Seconds + 1, SecondsFrac + SampleNr - TimerCounterMax);
    }
    return TimeStamp::Calc(Seconds, SecondsFrac + SampleNr);
  }
}
