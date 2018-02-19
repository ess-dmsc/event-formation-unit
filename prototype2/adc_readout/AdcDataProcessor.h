//
//  ADC_Readout.hpp
//  ADC_Data_Receiver
//
//  Created by Jonas Nilsson on 2017-10-17.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

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
