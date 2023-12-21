// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
//===----------------------------------------------------------------------===//

#pragma once

#include "readout/DataEventTypes.h"
#include <cstdint>
#include <memory>

using namespace std;

namespace Timepix3 {

class PixelDataEventBuilder;

// Definition of PixelDataEvent
struct PixelDataEvent {
  const uint16_t dCol;
  const uint16_t sPix;
  const uint8_t pix;
  const uint16_t ToT;
  const uint8_t fToA;
  const uint16_t toa;
  const uint32_t spidrTime;

  PixelDataEvent(uint16_t dCol, uint16_t sPix, uint8_t pix, uint16_t ToT,
                 uint8_t fToA, uint16_t toa, uint32_t spidrTime);
};
} // namespace Timepix3