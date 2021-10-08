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
#include <common/detector/Detector.h>
#include <common/kafka/Producer.h>
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
  /// \note There is probably no performance benefit running this on a separate
  /// thread.
  virtual void inputThread();

  /// \brief Process a data packet.
  void parsePacketWithStats(InData const &Packet, PacketParser &Parser);

  /// \brief The function that executes the code for parsing and processing the
  /// sample data.
  /// This function will return when Detector::runThreads is set to false.
  virtual void processingThread(Queue &DataModuleQueue,
                                std::shared_ptr<std::int64_t> EventCounter,
                                size_t QueueId, ChannelID Identifier);

  /// \brief Does on demand instantiation of Kafka producer.
  /// Used in order to simplify unit testing.
  virtual std::shared_ptr<Producer> getProducer();

  virtual std::shared_ptr<DelayLineProducer>
  getDelayLineProducer(OffsetTime UsedOffset);

  SamplingRun *getDataModuleFromQueue(ChannelID const Identifier);
  bool queueUpDataModule(SamplingRun *Data);

  std::map<ChannelID, std::unique_ptr<Queue>> DataModuleQueues{};

  std::map<std::string, TimeStampLocation> TimeStampLocationMap{
      {"Start", TimeStampLocation::Start},
      {"Middle", TimeStampLocation::Middle},
      {"End", TimeStampLocation::End}};

  /// \brief Counters that are used to store stats that are sent to Grafana.
  struct {
    /// \todo stat counter used as a timestamp, might be ok, but is
    /// a slight abuse of the intented use.
    std::int64_t CurrentTsMs = 0;
    std::int64_t InputBytesReceived = 0;
    std::int64_t ParseErrors = 0;
    std::int64_t ParserPacketsTotal = 0;
    std::int64_t ParserPacketsIdle = 0;
    std::int64_t ParserPacketsData = 0;
    std::int64_t ProcessingBufferFull = 0;
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

  OffsetTime TimestampOffset{OffsetTime::Offset::NONE};
};
