/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "DelayLineProcessing.h"
#include "PulseParameters.h"
#include <cmath>
#include <limits>

DelayLineProcessing::DelayLineProcessing(
    std::shared_ptr<DelayLineProducer> Prod, double const Threshold)
    : AdcDataProcessor(std::move(Prod)), TimestampThreshold(Threshold) {}

void DelayLineProcessing::processData(SamplingRun const &Data) {
  dynamic_cast<DelayLineProducer *>(ProducerPtr.get())
      ->addPulse(analyseSampleRun(Data, TimestampThreshold));
  dynamic_cast<DelayLineProducer *>(ProducerPtr.get())->addReferenceTimestamp(Data.ReferenceTimestamp);
}
