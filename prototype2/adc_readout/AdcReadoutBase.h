/// Copyright (C) 2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Adc readout detector class.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcBufferElements.h"
#include "AdcParse.h"
#include "AdcSettings.h"
#include "CircularBuffer.h"
#include "SampleProcessing.h"
#include <common/Detector.h>
#include <common/Producer.h>
#include <cstdint>
#include <mutex>

/// @brief Implements the code for the ADC detector module. Is a base of
/// AdcReadout in order to simplify unit testing.
class AdcReadoutBase : public Detector {
public:
  /// @param Settings EFU base command line settings.
  /// @param ReadoutSettings AdcReadout specific settings.
  AdcReadoutBase(BaseSettings const &Settings, AdcSettings &ReadoutSettings);
  AdcReadoutBase(const AdcReadoutBase &) = delete;
  AdcReadoutBase(const AdcReadoutBase &&) = delete;
  AdcReadoutBase &operator=(const AdcReadoutBase &) = delete;
  ~AdcReadoutBase() = default;

protected:
  using DataModulePtr = SpscBuffer::ElementPtr<SamplingRun>;
  using Queue = SpscBuffer::CircularBuffer<SamplingRun>;

  /// @brief Implements the thread doing the socket communication.
  /// This function will return when Detector::runThreads is set to false.
  /// @note There is probably no performance benefit runnign this on a seperate
  /// thread.
  virtual void inputThread();

  /// @brief The function that executes the code for parsing and processing the
  /// sample data.
  /// This function will return when Detector::runThreads is set to false.
  virtual void processingThread(Queue &DataModuleQueue);

  /// @brief Does on demand instatiation of Kafka producer.
  /// Used in order to simplify unit testing.
  virtual std::shared_ptr<Producer> getProducer();

  SamplingRun *GetDataModule(int Channel);
  bool QueueUpDataModule(SamplingRun *Data);

  std::vector<std::unique_ptr<Queue>> DataModuleQueues;

  /// @brief Used to keeps track of the global counter provided by the ADC
  /// hardware in order to figure out if a packet has been lost.
  std::uint16_t LastGlobalCount{0};

  std::map<std::string, TimeStampLocation> TimeStampLocationMap{
      {"Start", TimeStampLocation::Start},
      {"Middle", TimeStampLocation::Middle},
      {"End", TimeStampLocation::End}};

  /// @brief Counters that are used to store stats that are sent to Grafana.
  struct {
    std::int64_t current_ts_seconds = 0;
    std::int64_t input_bytes_received = 0;
    std::int64_t parser_errors = 0;
    std::int64_t parser_unknown_channel = 0;
    std::int64_t parser_packets_total = 0;
    std::int64_t parser_packets_idle = 0;
    std::int64_t parser_packets_data = 0;
    std::int64_t processing_buffer_full = 0;
    std::int64_t processing_packets_lost = -1; // This should be -1
  } AdcStats;

  std::shared_ptr<Producer> ProducerPtr;
  AdcSettings &ReadoutSettings;
  BaseSettings GeneralSettings;
  static const int MessageQueueSize = 100;
};
