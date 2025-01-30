// Copyright (C) 2023-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#pragma once

#include "common/reduction/Hit2D.h"
#include "common/reduction/Hit2DVector.h"
#include "geometry/Timepix3Geometry.h"
#include "handlers/PixelEventHandler.h"
#include <atomic>
#include <cstdint>
#include <dataflow/DataObserverTemplate.h>
#include <dto/TimepixDataTypes.h>
#include <future>
#include <memory>
#include <modules/timepix3/Counters.h>
#include <vector>

namespace Timepix3 {

// | 8b  |      12b        |     35b   |  4b   |  5b  |
// | 0x6 | Trigger counter | Timestamp | Stamp | Resv |

// clang-format off
#define TYPE_MASK 0xF000000000000000
#define TYPE_OFFS 60

// pixel type data
#define PIXEL_DCOL_MASK     0x0FE0000000000000
#define PIXEL_SPIX_MASK     0x001F800000000000
#define PIXEL_PIX_MASK      0x0000700000000000
#define PIXEL_TOA_MASK      0x00000FFFC0000000
#define PIXEL_TOT_MASK      0x000000003FF00000
#define PIXEL_FTOA_MASK     0x00000000000F0000
#define PIXEL_SPTIME_MASK   0x000000000000FFFF
#define PIXEL_DCOL_OFFSET   52
#define PIXEL_SPIX_OFFSET   45
#define PIXEL_PIX_OFFSET    44
#define PIXEL_TOA_OFFSET    30
#define PIXEL_TOT_OFFSET    20
#define PIXEL_FTOA_OFFSET   16

// TDC type data
#define TDC_TYPE_MASK             0x0F00000000000000
#define TDC_TRIGGERCOUNTER_MASK   0x00FFF00000000000
#define TDC_TIMESTAMP_MASK        0x00000FFFFFFFFE00
#define TDC_STAMP_MASK            0x00000000000001E0
#define TDC_TYPE_OFFSET           56
#define TDC_TRIGGERCOUNTER_OFFSET 44
#define TDC_TIMESTAMP_OFFSET      9
#define TDC_STAMP_OFFSET          5

// Global Timestamp type data
#define GLOBAL_TIMESTAMP_MASK     0x00FFFFFFFFFFFF00
#define GLOBAL_STAMP_MASK         0x00000000000000F0
#define GLOBAL_TIMESTAMP_OFFSET   8
#define GLOBAL_STAMP_OFFSET       4

#define EVR_READOUT_TYPE          1

#define EVR_READOUT_TYPE          1
// clang-format on

class DataParser
    : public Observer::DataEventObservable<timepixReadout::TDCReadout>,
      public Observer::DataEventObservable<timepixReadout::EVRReadout>,
      public Observer::DataEventObserver<timepixDTO::ESSGlobalTimeStamp> {
public:
  const unsigned int MaxReadoutsInPacket{500};

  DataParser(Counters &counters, std::shared_ptr<Timepix3Geometry> geometry);

  ~DataParser(){};

  void applyData(const timepixDTO::ESSGlobalTimeStamp &) override;

  Hit2DVector parseTPX(nonstd::span<uint64_t> &);

  inline int parseEVR(const char *buffer) {
    using namespace std::chrono;
    auto evrProcessingStart = steady_clock::now();

    timepixReadout::EVRReadout *Data =
        (timepixReadout::EVRReadout *)((char *)buffer);

    XTRACE(DATA, DEB,
           "Processed readout, packet type = %u, counter = %u, pulsetime "
           "seconds = %u, "
           "pulsetime nanoseconds = %u, previous pulsetime seconds = %u, "
           "previous pulsetime nanoseconds = %u",
           1, Data->counter, Data->pulseTimeSeconds, Data->pulseTimeNanoSeconds,
           Data->prevPulseTimeSeconds, Data->prevPulseTimeNanoSeconds);

    Observer::DataEventObservable<timepixReadout::EVRReadout>::publishData(
        *Data);
    Stats.EVRReadoutCounter++;

    auto evrProcessingEnd = steady_clock::now();
    Stats.EVRProcessingTimeUs +=
        duration_cast<microseconds>(evrProcessingEnd - evrProcessingStart)
            .count();

    return 1;
  }

  struct Counters &Stats;

private:
  std::optional<timepixDTO::ESSGlobalTimeStamp>
      lastEpochESSPulseTime;

  std::shared_ptr<Timepix3Geometry> Geometry;

  // Const expression
  static constexpr uint8_t PIXEL_READOUT_TYPE_CONST = 11;
  static constexpr uint8_t TDC_READOUT_TYPE_CONST = 6;

  static constexpr uint8_t TDC1_RISING_CONST = 15;
  static constexpr uint8_t TDC1_FALLING_CONST = 10;
  static constexpr uint8_t TDC2_RISING_CONST = 14;
  static constexpr uint8_t TDC2_FALLING_CONST = 11;

  /// \brief calculated the X coordinate from a const PixelDataEvent
  // Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
  // https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
  static inline uint32_t calcX(const timepixReadout::PixelReadout &Data) {
    uint32_t Col = static_cast<uint32_t>(Data.dCol) + Data.pix / 4;
    return Col;
  }

  /// \brief calculated the Y coordinate from a const PixelDataEvent
  // Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
  // https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
  static inline uint32_t calcY(const timepixReadout::PixelReadout &Data) {
    uint32_t Row = static_cast<uint32_t>(Data.sPix) + (Data.pix & 0x3);
    return Row;
  }

  Hit2D parsePixelReadout(const uint64_t &ReadoutData) const;

  void processTDCData(const uint64_t &readoutData) const;

  int processEVRData(const char *buffer, unsigned int size) const;
};

} // namespace Timepix3
