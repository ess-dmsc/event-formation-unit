// Copyright (C) 2023-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief The TimingEventHandler class handles timing events for the Timepix3
/// module.
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>
#include <chrono>
#include <common/kafka/EV44Serializer.h>
#include <common/utils/EfuUtils.h>
#include <cstdint>
#include <dataflow/DataObserverTemplate.h>
#include <dto/TimepixDataTypes.h>
#include <memory>
#include <timepix3/Counters.h>

#define TDC_MAX_TIMESTAMP_NS 107374182400

using namespace std;

namespace Timepix3 {

/**
 * @class TimingEventHandler
 * @brief Handles timing events for TDCReadout and EVRReadout data.
 *
 * This class is responsible for handling timing events related to TDCReadout
 * and EVRReadout data. It implements the Observer pattern as a
 * DataEventObserver for TDCReadout and EVRReadout, and as a DataEventObservable
 * for ESSGlobalTimeStamp.
 *
 * The class maintains a threshold value for the timing difference between TDC
 * and EVR events, and provides a method to check if the last timing difference
 * is lower than the threshold.
 *
 * The class also stores the last TDCDataEvent and EVRDataEvent received, and
 * provides methods to apply data to these events.
 *
 * @note The threshold value is currently hard-coded and can be moved to a
 * configuration file in the future.
 *
 * @see Observer::DataEventObserver
 * @see Observer::DataEventObservable
 * @see timepixReadout::TDCReadout
 * @see timepixReadout::EVRReadout
 * @see timepixDTO::ESSGlobalTimeStamp
 */
class TimingEventHandler
    : public Observer::DataEventObserver<timepixReadout::TDCReadout>,
      public Observer::DataEventObserver<timepixReadout::EVRReadout>,
      public Observer::DataEventObservable<timepixDTO::ESSGlobalTimeStamp> {

private:
  /// \todo move this into configuration file
  const milliseconds THRESHOLD_MS = milliseconds(20);

  Counters &statCounters;
  
  const nanoseconds FrequencyPeriodNs;


  unique_ptr<timepixDTO::TDCDataEvent> lastTDCData;
  unique_ptr<timepixDTO::EVRDataEvent> lastEVRData;

  uint32_t tdcRepetitionFrequency{static_cast<uint32_t>(FrequencyPeriodNs.count())};

  /**
   * @brief Checks if the timing difference between the last TDCDataEvent and
   * EVRDataEvent is lower than the threshold.
   *
   * @return true if the timing difference is lower than the threshold, false
   * otherwise.
   */
  inline bool isLastTimingDiffLowerThenThreshold() {
    if (lastEVRData == nullptr || lastTDCData == nullptr) {
      return false;
    }

    auto arrivalDiff =
      abs(duration_cast<nanoseconds>(lastTDCData->arrivalTimestamp -
                      lastEVRData->arrivalTimestamp)
          .count());

    return arrivalDiff <= duration_cast<nanoseconds>(THRESHOLD_MS).count();
  }

public:
  /**
   * @brief Constructs a TimingEventHandler object.
   *
   * @param statCounters The reference to the Counters object for tracking
   * statistics.
   */
  TimingEventHandler(Counters &statCounters, const int &Frequency = 14) :
      statCounters(statCounters),
      FrequencyPeriodNs(efutils::hzToNanoseconds(Frequency)) {}

  /**
   * @brief Destroys the TimingEventHandler object.
   */
  virtual ~TimingEventHandler(){};

  /**
   * @brief Applies the data from a TDCReadout object.
   *
   * @param tdcReadout The TDCReadout object containing the data to be applied.
   */
  void applyData(const timepixReadout::TDCReadout &tdcReadout) override;

  /**
   * @brief Applies the data from an EVRReadout object.
   *
   * @param evrReadout The EVRReadout object containing the data to be applied.
   */
  void applyData(const timepixReadout::EVRReadout &evrReadout) override;
};

} // namespace Timepix3