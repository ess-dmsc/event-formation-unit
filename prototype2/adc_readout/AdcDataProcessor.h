/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Some general utility code for implementing ADC data processing.
 */

#pragma once

#include "AdcParse.h"
#include <common/Producer.h>
#include <memory>

/// @brief Base class for the AdcReadout data processors.
/// Classes that inherit from AdcDataProcessor must implement AdcDataProcessor::processPacket().
class AdcDataProcessor {
public:
  AdcDataProcessor(std::shared_ptr<ProducerBase> Prod);
  virtual ~AdcDataProcessor() = default;
  
  /// @brief Pure virtual function that must be implemented in order to process parsed data.
  virtual void processPacket(const PacketData &Data) = 0;

protected:
  std::shared_ptr<ProducerBase> ProducerPtr;
};
