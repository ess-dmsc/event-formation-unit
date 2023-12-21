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
#include "common/utils/UnitConverter.h"
#include "dataflow/DataObserverTemplate.h"
#include "readout/DataEventTypes.h"
#include <memory>

namespace Timepix3 {

using namespace Observer;
using namespace std;
using namespace efutils;
using namespace chrono;

class TimingEventHandler : public DataEventObserver<shared_ptr<TDCDataEvent>>,
                           public DataEventObserver<shared_ptr<EVRDataEvent>>,
                           public DataEventObservable<uint64_t> {

private:
  const milliseconds THRESHOLD_MS = nsToMilliseconds(DEFAULT_FREQUENCY_NS / 2);

  Counters &statCounters;
  const EV44Serializer &serializer;
  // ToDo verify that all the data has to be stored or only some processed data
  shared_ptr<TDCDataEvent> lastTDCData;
  shared_ptr<EVRDataEvent> lastEVRData;

    DataEventObservable<EpochESSPulseTime>& epochESSPulseTimeObservable;

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
        Counters &statCounters, EV44Serializer &serializer,
        DataEventObservable<EpochESSPulseTime>& epochESSPulseTimeObservable)
        : statCounters(statCounters), serializer(serializer),
          epochESSPulseTimeObservable(epochESSPulseTimeObservable) {}

    virtual ~TimingEventHandler(){};

    void applyData(const shared_ptr<TDCDataEvent> &newData) override;
    void applyData(const shared_ptr<EVRDataEvent> &newData) override;

    shared_ptr<TDCDataEvent> getLastTDCData() const {
      return lastTDCData;
    }

    shared_ptr<EVRDataEvent> getLastEVRData() const {
      return lastEVRData;
    }
  };

  } // namespace Timepix3
