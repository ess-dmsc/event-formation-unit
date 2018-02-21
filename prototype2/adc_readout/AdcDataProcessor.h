/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Some general utility code for implementing ADC data processing.
 */

#pragma once

#include <common/Producer.h>
#include <memory>
#include "AdcParse.h"

class AdcDataProcessor {
public:
  AdcDataProcessor(std::shared_ptr<Producer> Prod);
  virtual void operator()(const PacketData &Data) = 0;
protected:
  std::shared_ptr<Producer> ProducerPtr;
};

namespace TimeStamp {
  std::uint64_t CalcSample(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac, const std::uint32_t &SampleNr);
  
  std::uint64_t Calc(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac);
  
  //Note: This function might be significantly (a lot) slower than TimeStamp::Calc() for some cases. The reverse is also true.
  std::uint64_t CalcFast(const std::uint32_t &Seconds, const std::uint32_t &SecondsFrac);
}
