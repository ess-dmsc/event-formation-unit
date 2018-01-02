//
//  ADC_Readout.hpp
//  ADC_Data_Receiver
//
//  Created by Jonas Nilsson on 2017-10-17.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#pragma once

#include <mutex>
#include <cstdint>
#include <common/Detector.h>
#include <common/Producer.h>
#include "CircularBuffer.h"
#include "AdcBufferElements.h"
#include "AdcParse.h"
#include "PeakFinder.h"

class AdcReadout : public Detector {
public:
  AdcReadout(BaseSettings Settings);
  AdcReadout(const AdcReadout&) = delete;
  AdcReadout(const AdcReadout&&) = delete;
  ~AdcReadout() = default;
protected:
  void inputThread();
  void parsingThread();
  using ElementPtr = SpscBuffer::ElementPtr<InData>;
  using Queue = SpscBuffer::CircularBuffer<InData>;
  Queue toParsingQueue;
  std::uint16_t LastGlobalCount;
  
  std::unique_ptr<AdcDataProcessor> Processor;
  
//  std::function<void(PacketData&)> ProcessingFunction;
  
  struct {
    std::int64_t input_bytes_received = 0;
    std::int64_t parser_errors_unknown = 0;
    std::int64_t parser_errors_feedf00d = 0;
    std::int64_t parser_errors_filler = 0;
    std::int64_t parser_errors_beefcafe = 0;
    std::int64_t parser_errors_dlength = 0;
    std::int64_t parser_errors_abcd = 0;
    std::int64_t parser_errors_hlength = 0;
    std::int64_t parser_errors_type = 0;
    std::int64_t parser_errors_ilength = 0;
    std::int64_t parser_packets_total = 0;
    std::int64_t parser_packets_idle = 0;
    std::int64_t parser_packets_data = 0;
    std::int64_t parser_packets_error = 0;
    std::int64_t processing_packets_lost = 0;
  } AdcStats;
  void addParserError(ParserException::Type ExceptionType);
  
  std::shared_ptr<Producer> ProducerPtr;
};

class ADC_Readout_Factory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings Settings) {
    return std::shared_ptr<Detector>(new AdcReadout(Settings));
  }
};
