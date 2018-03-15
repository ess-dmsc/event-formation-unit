/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Some general utility code for implementing ADC data processing.
 */

#pragma once

#include "AdcParse.h"
#include <common/Producer.h>
#include <memory>

class AdcDataProcessor {
public:
  AdcDataProcessor(std::shared_ptr<ProducerBase> Prod);
  virtual ~AdcDataProcessor() = default;
  virtual void processPacket(const PacketData &Data) = 0;

protected:
  std::shared_ptr<ProducerBase> ProducerPtr;
};
