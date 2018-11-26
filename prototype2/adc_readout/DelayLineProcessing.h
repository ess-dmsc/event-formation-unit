/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple peak finding algorithm implementation header.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcDataProcessor.h"
#include "AdcParse.h"
#include "DelayLineProducer.h"
#include <memory>

/// \brief Extract pulse parameters and send them for processing.
class DelayLineProcessing : public AdcDataProcessor {
public:
  /// \param[in] Prod A shared pointer to the Kafka producer that handles data
  /// production.
  DelayLineProcessing(std::shared_ptr<DelayLineProducer> Prod);

  /// \brief Extract data of importance from sampling run.
  virtual void processData(SamplingRun const &Data) override;
};

/// \brief Implements the data extraction information as  pure function.
///
/// \param[in] SampleRun A vector of samples to find the peak in.
/// \return The results of the peak finding algorithm.
PulseParameters analyseSampleRun(SamplingRun const &Run);
