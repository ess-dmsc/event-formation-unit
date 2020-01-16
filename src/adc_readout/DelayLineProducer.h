/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple peak finding algorithm implementation header.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcSettings.h"
#include "AdcTimeStamp.h"
#include "DelayLineEventFormation.h"
#include "EventSerializer.h"
#include "PulseParameters.h"
#include <atomic>
#include <common/Producer.h>
#include <logical_geometry/ESSGeometry.h>
#include <mutex>
#include <queue>
#include <thread>

/// \brief Kafka producer class intended only for delay line data production.
/// Implements some glue logic and serialisation.
class DelayLineProducer : public Producer {
public:
  /// \brief Sets up the Kafka producer and initialises the event formation
  /// code.
  ///
  /// This class will start a thread for processing and producing the events
  /// that are added using DelayLineProducer::addPulse().
  ///
  /// \param[in] Broker Address and port pf the broker, e.g. localhost:9092.
  /// \param[in] Topic Kafka topic to which the event data will be published.
  /// \param[in] EfuSettings The required settings for setting up the delay line
  /// position calculation.
  DelayLineProducer(std::string Broker, std::string Topic,
                    AdcSettings EfuSettings, OffsetTime UsedOffset);

  /// \brief Stop processing thread and deallocate resources.
  ~DelayLineProducer();

  /// \brief Add pulse to queue for processing by the processing thread.
  /// \param[in] Pulse Pulse parameters of the registered pulse.
  void addPulse(PulseParameters const Pulse);

  void addReferenceTimestamp(TimeStamp const &ReferenceTimestamp);

  std::int64_t &getNrOfEvents() { return EventCounter; }

protected:
  /// \brief Serialize the event produced by one or more pulses.
  /// \param[in] Event Holds postion, timestamp and amplitude of event. For some
  /// types of event, the amplitude is meaningless.
  virtual void serializeAndSendEvent(DelayLineEvent const &Event);

  std::int64_t EventCounter{0};

  /// \brief Processing thread. Does not return unless
  /// DelayLineProducer::RunThread is set to false.
  void pulseProcessingFunction();
  using Producer::produce;
  AdcSettings Settings;
  std::thread PulseProcessingThread;
  std::mutex PulseMutex;
  std::queue<PulseParameters> PulseQueue{};
  std::atomic_bool RunThread{true};
  ESSGeometry essgeometry{512, 512, 1, 1};
  EventSerializer Serializer;
  std::uint64_t CurrentReferenceTimestamp{0};
};
