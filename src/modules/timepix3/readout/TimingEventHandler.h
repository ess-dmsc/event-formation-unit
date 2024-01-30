// Copyright (C) 2023-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for TDC and EVR timing event observers. Responsible
//         to follow up different timing events and calculate the global time
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV44Serializer.h>
#include <common/utils/EfuUtils.h>
#include <dataflow/DataObserverTemplate.h>
#include <memory>
#include <readout/TimepixDataTypes.h>
#include <timepix3/Counters.h>

#define TDC_MAX_TIMESTAMP_NS 107374182400

using namespace std;

namespace Timepix3 {

class TimingEventHandler
    : public Observer::DataEventObserver<timepixReadout::TDCReadout>,
      public Observer::DataEventObserver<timepixReadout::EVRReadout>,
      public Observer::DataEventObservable<timepixDTO::ESSGlobalTimeStamp> {

private:
/// \todo move this into configuration file
  const milliseconds THRESHOLD_MS = milliseconds(20);

  Counters &statCounters;

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

  TimingEventHandler(Counters &statCounters) : statCounters(statCounters) {}

  virtual ~TimingEventHandler(){};

  void applyData(const timepixReadout::TDCReadout &tdcReadout) override;
  void applyData(const timepixReadout::EVRReadout &evrReadout) override;
};

} // namespace Timepix3