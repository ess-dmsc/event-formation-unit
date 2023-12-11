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

#include <Counters.h>
#include <chrono>
#include <common/utils/UnitConverter.h>
#include <dataflow/DataObserverTemplate.h>
#include <memory>
#include <readout/DataEventTypes.h>

namespace Timepix3 {

using namespace Observer;
using namespace std;
using namespace efutils;
using namespace chrono;

class TimingEventHandler : public DataEventObserver<TDCDataEvent>,
                           public DataEventObserver<EVRDataEvent> {

private:
  const milliseconds THRESHOLD_MS = nsToMilliseconds(DEFAULT_FREQUENCY_NS/2);

  Counters &statCounters;
  // ToDo verify that all the data has to be stored or only some processed data
  shared_ptr<TDCDataEvent> lastTdcData;
  shared_ptr<EVRDataEvent> lastEVRData;
  uint64_t nextTDCTimeStamp;
  shared_ptr<TDCDataEvent> lastTDCPair;

  uint32_t tdcRepetitionFrequency{DEFAULT_FREQUENCY_NS};

  bool lookingForNextTDC{false};

  inline bool isLastTimingDiffLowerThenThreshold() {
    auto arrivalDiff =
        abs(duration_cast<milliseconds>(lastTdcData->arrivalTimestamp -
                                        lastEVRData->arrivalTimestamp)
                .count());

    return arrivalDiff <= THRESHOLD_MS.count();
  }

  void updateTDCFrequency(const TDCDataEvent &);

public:
  static const uint32_t DEFAULT_FREQUENCY_NS;

  TimingEventHandler(Counters &statCounters) : statCounters(statCounters) {}
  virtual ~TimingEventHandler(){};

  void applyData(const TDCDataEvent &newData) override;
  void applyData(const EVRDataEvent &newData) override;

  uint64_t getLastTDCTimestamp() const;
  uint32_t getTDCFrequency() const;
  const shared_ptr<TDCDataEvent> getLastTDCPair() const;
  const shared_ptr<TDCDataEvent> getLastTdcEvent() const;
  const shared_ptr<EVRDataEvent> getLastEvrEvent() const;
};

} // namespace Timepix3