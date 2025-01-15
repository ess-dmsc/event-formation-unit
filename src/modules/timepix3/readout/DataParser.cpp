// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#include "common/reduction/Hit2D.h"
#include "common/reduction/Hit2DVector.h"
#include "geometry/Timepix3Geometry.h"
#include "handlers/PixelEventHandler.h"
#include <algorithm>
#include <chrono>
#include <common/debug/Trace.h>
#include <common/memory/ThreadPool.hpp>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <timepix3/readout/DataParser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace timepixReadout;

DataParser::DataParser(struct Counters &counters,
                       std::shared_ptr<Timepix3Geometry> geometry,
                       Timepix3::PixelEventHandler &pixelEventHandler)
    : Stats(counters), Geometry(geometry),
      PixelEventHandler(pixelEventHandler) {}

int DataParser::parse(const char *Buffer, unsigned int Size) {
  using namespace std::chrono;
  auto start = steady_clock::now();

  XTRACE(DATA, DEB, "parsing data, size is %u", Size);

  // 1. Check for EVR
  if (Size == sizeof(struct EVRReadout)) {
    auto evrProcessingStart = steady_clock::now();
    EVRReadout *Data = (EVRReadout *)((char *)Buffer);

    if (Data->type == EVR_READOUT_TYPE) {
      // ...existing EVR processing code...
      XTRACE(DATA, DEB,
             "Processed readout, packet type = %u, counter = %u, pulsetime "
             "seconds = %u, "
             "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
             "previous pulsetime nanoseconds = %u",
             1, Data->counter, Data->pulseTimeSeconds,
             Data->pulseTimeNanoSeconds, Data->prevPulseTimeSeconds,
             Data->prevPulseTimeNanoSeconds);

      DataEventObservable<EVRReadout>::publishData(*Data);
      Stats.EVRReadoutCounter++;

      auto evrProcessingEnd = steady_clock::now();
      Stats.EVRProcessingTimeMs +=
          duration_cast<microseconds>(evrProcessingEnd - evrProcessingStart)
              .count();

      return 1;
    }
  }

  // 2. Create vector that overlaps buffer data without copying
  const uint64_t *dataStart = reinterpret_cast<const uint64_t *>(Buffer);
  const size_t numElements = Size / sizeof(uint64_t);
  const nonstd::span<const uint64_t> readoutData(dataStart,
                                                 dataStart + numElements);

  // Create local thread pool
  ThreadPool threadPool = ThreadPool();
  
  // Process TDC messages using local thread pool
  auto tdcFuture = threadPool.enqueue(&DataParser::processTDCData, this, readoutData);

  PixelEventHandler.Hits.clear();              // Clear previous hits
  PixelEventHandler.Hits.reserve(numElements); // Pre-allocate space

  // Create vector of futures for parallel processing
  auto startPixelReadoutProcessing = steady_clock::now();
  std::vector<std::future<Hit2D>> futures;
  futures.reserve(numElements);

  if (!lastEpochESSPulseTime) {
    Stats.NoGlobalTime++;
    return 0;
  }

  // Use local thread pool for pixel processing
  for (const auto& data : readoutData) {
    futures.push_back(threadPool.enqueue(&DataParser::parsePixelReadout, this, data));
  }

  auto stopPixelReadoutProcessing = steady_clock::now();
  Stats.PixelFuturesTimeMs +=
      duration_cast<microseconds>(stopPixelReadoutProcessing -
                                  startPixelReadoutProcessing)
          .count();
  // Collect results

  auto hitVectorCreationStart = steady_clock::now();
  for (auto &future : futures) {
    try {
      PixelEventHandler.Hits.emplace_back(future.get());
    } catch (const std::exception &e) {
      Stats.PixelErrors++;
    }
  }

  auto hitVectorCreationEnd = steady_clock::now();
  Stats.HitVectorCreationTimeMs +=
      duration_cast<microseconds>(hitVectorCreationEnd - hitVectorCreationStart)
          .count();

  tdcFuture.wait();

  auto end = steady_clock::now();
  Stats.TotalParseTimeMs += duration_cast<microseconds>(end - start).count();

  return futures.size(); // Return number of processed pixel readouts
}

// Make parsePixelReadout const to ensure thread safety
Hit2D DataParser::parsePixelReadout(const uint64_t ReadoutData) const {
  uint8_t ReadoutType = (ReadoutData & TYPE_MASK) >> TYPE_OFFS;

  if (ReadoutType != PIXEL_READOUT_TYPE_CONST) {
    throw std::runtime_error("Readout data is not a pixel readout");
  }

  if (!lastEpochESSPulseTime) {
    throw std::runtime_error("No epoch ESS pulse time available");
  }

  PixelReadout pixelDataEvent(
      (ReadoutData & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFSET,
      (ReadoutData & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFSET,
      (ReadoutData & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFSET,
      (ReadoutData & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFSET,
      (ReadoutData & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFSET,
      (ReadoutData & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFSET,
      ReadoutData & PIXEL_SPTIME_MASK);

  // if (Geometry->validateData(pixelDataEvent)) {
  //   throw std::runtime_error("Invalid pixel data");
  // }
  // // Calculate TOF in ns
  uint16_t X = calcX(pixelDataEvent);
  uint16_t Y = calcY(pixelDataEvent);

  uint64_t pixelClockTime =
      409600 * static_cast<uint64_t>(pixelDataEvent.spidrTime) +
      25 * static_cast<uint64_t>(pixelDataEvent.toa) -
      1.5625 * static_cast<uint64_t>(pixelDataEvent.fToA);

  uint64_t pixelGlobalTimeStamp = 0;
  // happens in case of pixel clock reset between two tdc
  if (lastEpochESSPulseTime->tdcClockInPixelTime > pixelClockTime) {

    // Calculate time until reset from the last tdc time
    uint64_t timeUntilReset =
        PIXEL_MAX_TIMESTAMP_NS - lastEpochESSPulseTime->tdcClockInPixelTime;

    pixelGlobalTimeStamp = lastEpochESSPulseTime->pulseTimeInEpochNs +
                           timeUntilReset + pixelClockTime;
  } else {
    uint64_t tofInPixelTime =
        pixelClockTime - lastEpochESSPulseTime->tdcClockInPixelTime;
    pixelGlobalTimeStamp =
        lastEpochESSPulseTime->pulseTimeInEpochNs + tofInPixelTime;
  }

  // Assuming Hit2D constructor parameters are (x, y, weight, time)
  return {pixelGlobalTimeStamp - lastEpochESSPulseTime->pulseTimeInEpochNs, X,
          Y, pixelDataEvent.ToT};
};

void DataParser::applyData(
    const timepixDTO::ESSGlobalTimeStamp &epochEssPulseTime) {

  lastEpochESSPulseTime =
      std::make_unique<timepixDTO::ESSGlobalTimeStamp>(epochEssPulseTime);
}

void DataParser::processTDCData(
    const nonstd::span<const uint64_t> &readoutData) const {
  using namespace std::chrono;

  for (const auto &data : readoutData) {
    uint8_t ReadoutType = (data & TYPE_MASK) >> TYPE_OFFS;
    if (ReadoutType == TDC_READOUT_TYPE_CONST) {
      auto tdcProcessingStart = steady_clock::now();

      TDCReadout tdcReadout((data & TDC_TYPE_MASK) >> TDC_TYPE_OFFSET,
                            (data & TDC_TRIGGERCOUNTER_MASK) >>
                                TDC_TRIGGERCOUNTER_OFFSET,
                            (data & TDC_TIMESTAMP_MASK) >> TDC_TIMESTAMP_OFFSET,
                            (data & TDC_STAMP_MASK) >> TDC_STAMP_OFFSET);

      Stats.TDCReadoutCounter++;

      if (tdcReadout.type == TDC1_RISING_CONST) {
        Stats.TDC1RisingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC1_FALLING_CONST) {
        Stats.TDC1FallingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC2_RISING_CONST) {
        Stats.TDC2RisingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else if (tdcReadout.type == TDC2_FALLING_CONST) {
        Stats.TDC2FallingReadouts++;
        DataEventObservable<TDCReadout>::publishData(tdcReadout);
      } else {
        Stats.UnknownTDCReadouts++;
      }

      auto tdcProcessingEnd = steady_clock::now();
      Stats.TDCProcessingTimeMs +=
          duration_cast<microseconds>(tdcProcessingEnd - tdcProcessingStart)
              .count();
    }
  }
}

} // namespace Timepix3
