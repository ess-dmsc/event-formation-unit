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
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace timepixReadout;

DataParser::DataParser(struct Counters &counters,
                       std::shared_ptr<Timepix3Geometry> geometry)
    : Stats(counters), Geometry(geometry) {}

Hit2DVector DataParser::parseTPX(std::vector<uint64_t> &readoutData) {
  using namespace std::chrono;

  Hit2DVector hits;
  hits.reserve(readoutData.size());

  for (const auto &data : readoutData) {
    try {
      if ((data & TYPE_MASK) >> TYPE_OFFS == TDC_READOUT_TYPE_CONST) {
        processTDCData(data);
        continue;
      } else if ((data & TYPE_MASK) >> TYPE_OFFS != PIXEL_READOUT_TYPE_CONST) {
        Stats.UndefinedReadoutCounter++;
        continue;
      } else {
        if (!lastEpochESSPulseTime) {
          Stats.NoGlobalTime++;
          continue;
        }
        // Process pixel readout and measure processing time
        auto startPixelReadoutProcessing = steady_clock::now();

        hits.emplace_back(parsePixelReadout(data));

        auto stopPixelReadoutProcessing = steady_clock::now();
        Stats.PixelFuturesTimeUs +=
            duration_cast<microseconds>(stopPixelReadoutProcessing -
                                        startPixelReadoutProcessing)
                .count();
      }
    } catch (const std::exception &e) {
      Stats.PixelErrors++;
    }
  }

  return hits;
}

// Make parsePixelReadout const to ensure thread safety
Hit2D DataParser::parsePixelReadout(const uint64_t &ReadoutData) const {

  Stats.PixelReadouts++;

  if (!lastEpochESSPulseTime) {
    throw std::runtime_error("No epoch ESS pulse time available");
    Stats.InvalidPixelReadout++;
  }

  PixelReadout pixelDataEvent(
      (ReadoutData & PIXEL_DCOL_MASK) >> PIXEL_DCOL_OFFSET,
      (ReadoutData & PIXEL_SPIX_MASK) >> PIXEL_SPIX_OFFSET,
      (ReadoutData & PIXEL_PIX_MASK) >> PIXEL_PIX_OFFSET,
      (ReadoutData & PIXEL_TOA_MASK) >> PIXEL_TOA_OFFSET,
      (ReadoutData & PIXEL_TOT_MASK) >> PIXEL_TOT_OFFSET,
      (ReadoutData & PIXEL_FTOA_MASK) >> PIXEL_FTOA_OFFSET,
      ReadoutData & PIXEL_SPTIME_MASK);

  /// \todo Move Geometry back into parser and perform validation here
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

  lastEpochESSPulseTime.emplace(epochEssPulseTime);
}

void DataParser::processTDCData(const uint64_t &readoutData) const {
  using namespace std::chrono;
  auto tdcProcessingStart = steady_clock::now();

  TDCReadout tdcReadout(
      (readoutData & TDC_TYPE_MASK) >> TDC_TYPE_OFFSET,
      (readoutData & TDC_TRIGGERCOUNTER_MASK) >> TDC_TRIGGERCOUNTER_OFFSET,
      (readoutData & TDC_TIMESTAMP_MASK) >> TDC_TIMESTAMP_OFFSET,
      (readoutData & TDC_STAMP_MASK) >> TDC_STAMP_OFFSET);

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
  Stats.TDCProcessingTimeUs +=
      duration_cast<microseconds>(tdcProcessingEnd - tdcProcessingStart)
          .count();
}

} // namespace Timepix3
