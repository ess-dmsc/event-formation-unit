/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Adc readout detector class.
 */
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

class AdcReadoutCore : public Detector {
public:
  AdcReadoutCore(BaseSettings Settings, AdcSettingsStruct &AdcSettings);
  AdcReadoutCore(const AdcReadoutCore &) = delete;
  AdcReadoutCore(const AdcReadoutCore &&) = delete;
  ~AdcReadoutCore() = default;

protected:
  virtual void inputThread();
  virtual void parsingThread();
  virtual std::shared_ptr<Producer> getProducer();
  using ElementPtr = SpscBuffer::ElementPtr<InData>;
  using Queue = SpscBuffer::CircularBuffer<InData>;
  Queue toParsingQueue;
  std::uint16_t LastGlobalCount{0};

  std::vector<std::unique_ptr<AdcDataProcessor>> Processors;

  struct {
    std::int64_t input_bytes_received = 0;
    std::int64_t parser_errors = 0;
    std::int64_t parser_packets_total = 0;
    std::int64_t parser_packets_idle = 0;
    std::int64_t parser_packets_data = 0;
    std::int64_t parser_packets_stream = 0;
    std::int64_t processing_packets_lost = -1; // This should be -1
  } AdcStats;

  std::shared_ptr<Producer> ProducerPtr;
  AdcSettingsStruct &AdcSettings;
  BaseSettings GeneralSettings;
};
