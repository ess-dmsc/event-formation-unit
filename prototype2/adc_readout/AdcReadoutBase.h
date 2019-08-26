/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
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
#include "DelayLineProcessing.h"
#include "SampleProcessing.h"
#include <asio.hpp>
#include <common/Detector.h>
#include <common/Producer.h>
#include <cstdint>
#include <mutex>

/// \brief Implements the code for the ADC detector module. Is a base of
/// AdcReadout in order to simplify unit testing.
/// \note This class will lazily start threads for processing data as they are
/// needed. Thus if no data is received, only one thread (the input thread) will
/// be started.
class AdcReadoutBase : public Detector {
public:
  /// \param Settings EFU base command line settings.
  /// \param ReadoutSettings AdcReadout specific settings.
  AdcReadoutBase(BaseSettings const &Settings,
                 AdcSettings const &ReadoutSettings);
  AdcReadoutBase(const AdcReadoutBase &) = delete;
  AdcReadoutBase(const AdcReadoutBase &&) = delete;
  AdcReadoutBase &operator=(const AdcReadoutBase &) = delete;
  ~AdcReadoutBase() = default;

  void stopThreads() override;

protected:
  using DataModulePtr = SpscBuffer::ElementPtr<SamplingRun>;
  using Queue = SpscBuffer::CircularBuffer<SamplingRun>;

  /// \brief Implements the thread doing the socket communication.
  /// This function will return when Detector::runThreads is set to false.
  /// \note There is probably no performance benefit running this on a seperate
  /// thread.
  virtual void inputThread();

  void packetFunction(InData const &Packet, PacketParser &Parser);

  /// \brief The function that executes the code for parsing and processing the
  /// sample data.
  /// This function will return when Detector::runThreads is set to false.
  virtual void processingThread(Queue &DataModuleQueue,
                                std::shared_ptr<std::int64_t> EventCounter);

  /// \brief Does on demand instatiation of Kafka producer.
  /// Used in order to simplify unit testing.
  virtual std::shared_ptr<Producer> getProducer();

  virtual std::shared_ptr<DelayLineProducer> getDelayLineProducer();

  SamplingRun *GetDataModule(ChannelID const Identifier);
  bool QueueUpDataModule(SamplingRun *Data);

  std::map<ChannelID, std::unique_ptr<Queue>> DataModuleQueues{};

  std::map<std::string, TimeStampLocation> TimeStampLocationMap{
      {"Start", TimeStampLocation::Start},
      {"Middle", TimeStampLocation::Middle},
      {"End", TimeStampLocation::End}};

  /// \brief Counters that are used to store stats that are sent to Grafana.
  struct {
    std::int64_t current_ts_sec = 0;
    std::int64_t current_ts_alt_sec = 0;
    std::int64_t input_bytes_received = 0;
    std::int64_t parser_errors = 0;
    std::int64_t parser_packets_total = 0;
    std::int64_t parser_packets_idle = 0;
    std::int64_t parser_packets_data = 0;
    std::int64_t processing_buffer_full = 0;
    std::int64_t processing_packets_lost = -1; // This should be -1
  } AdcStats;

  std::shared_ptr<Producer> ProducerPtr{nullptr};
  std::shared_ptr<DelayLineProducer> DelayLineProducerPtr{nullptr};
  AdcSettings ReadoutSettings;
  BaseSettings GeneralSettings;
  const int MessageQueueSize = 100;
  std::shared_ptr<asio::io_service> Service;
  asio::io_service::work Worker;
  std::mutex ProducerMutex;
  std::mutex DelayLineProducerMutex;
};
