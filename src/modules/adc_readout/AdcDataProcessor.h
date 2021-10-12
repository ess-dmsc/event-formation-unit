/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Some general utility code for implementing ADC data processing.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcParse.h"
#include <common/kafka/Producer.h>
#include <memory>

/// \brief Base class for the AdcReadout data processors.
/// Classes that inherit from AdcDataProcessor must implement
/// AdcDataProcessor::processPacket().
class AdcDataProcessor {
public:
  explicit AdcDataProcessor(std::shared_ptr<ProducerBase> Prod);
  virtual ~AdcDataProcessor() = default;

  /// \brief Pure virtual function that must be implemented in order to process
  /// parsed data.
  virtual void processData(SamplingRun const &Data) = 0;

protected:
  std::shared_ptr<ProducerBase> ProducerPtr;
};
