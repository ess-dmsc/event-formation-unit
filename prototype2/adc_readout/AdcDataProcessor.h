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
  AdcDataProcessor(std::shared_ptr<ProducerBase> Prod);
  virtual ~AdcDataProcessor();
  virtual void operator()(const PacketData &Data) = 0;
protected:
  std::shared_ptr<ProducerBase> ProducerPtr;
};
