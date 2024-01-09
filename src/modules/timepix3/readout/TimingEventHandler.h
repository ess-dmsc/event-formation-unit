// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for TDC and EVR timing event observers. Responsible
//         to follow up different timing events and syncronize the two timing
//         (ESS time and camera time) domains. Also this class provides timing
//         information for other timing interested objects.
//===----------------------------------------------------------------------===//

#pragma once

#include "Counters.h"
#include "common/kafka/EV44Serializer.h"
#include "common/utils/EfuUtils.h"
#include "dataflow/DataObserverTemplate.h"
#include "readout/TimepixDataTypes.h"
#include <memory>

#define TDC_CLOCK_BIN_NS 3.125
#define TDC_FINE_CLOCK_BIN_NS 0.26
#define TDC_MAX_TIMESTAMP_NS 107.3741824 * 1e9

using namespace std;

namespace Timepix3 {

class TimingEventHandler
    : public Observer::DataEventObserver<timepixReadout::TDCReadout>,
      public Observer::DataEventObserver<timepixReadout::EVRReadout>,
      public Observer::DataEventObservable<timepixDTO::ESSGlobalTimeStamp> {

private:
  const milliseconds THRESHOLD_MS =
      efutils::nsToMilliseconds(DEFAULT_FREQUENCY_NS / 2);

  Counters &statCounters;
  const EV44Serializer &serializer;

  unique_ptr<timepixDTO::TDCDataEvent> lastTDCData;
  unique_ptr<timepixDTO::EVRDataEvent> lastEVRData;

  uint32_t tdcRepetitionFrequency{DEFAULT_FREQUENCY_NS};

  inline bool isLastTimingDiffLowerThenThreshold() {
    if (lastEVRData == nullptr || lastTDCData == nullptr) {
      return false;
    }

    auto arrivalDiff =
        abs(duration_cast<milliseconds>(lastTDCData->arrivalTimestamp -
                                        lastEVRData->arrivalTimestamp)
                .count());

    return arrivalDiff <= THRESHOLD_MS.count();
  }

public:
  static const uint32_t DEFAULT_FREQUENCY_NS;

  TimingEventHandler(
      Counters &statCounters, EV44Serializer &serializer)
      : statCounters(statCounters), serializer(serializer) {}

  virtual ~TimingEventHandler(){};

  void applyData(const timepixReadout::TDCReadout &newData) override;
  void applyData(const timepixReadout::EVRReadout &evrReadout) override;
};

} // namespace Timepix3