// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
//===----------------------------------------------------------------------===//

#include "PixelDataEvent.h"
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace Timepix3 {

// Implementation of PixelDataEventBuilder setter methods
// PixelDataEventBuilder &PixelDataEventBuilder::setDCol(const uint16_t &value) {
//   dCol = std::make_unique<uint16_t>(value);
//   return *this;
// }

// PixelDataEventBuilder &PixelDataEventBuilder::setSPix(const uint16_t &value) {
//   sPix = std::make_unique<uint16_t>(value);
//   return *this;
// }

// PixelDataEventBuilder &PixelDataEventBuilder::setPix(const uint8_t &value) {
//   pix = std::make_unique<uint8_t>(value);
//   return *this;
// }

// PixelDataEventBuilder &PixelDataEventBuilder::setToT(const uint16_t &value) {
//   ToT = std::make_unique<uint16_t>(value);
//   return *this;
// }

// PixelDataEventBuilder &PixelDataEventBuilder::calculateGlobalTime(
//     const EpochESSPulseTime &lastEpochPulseTime, const uint16_t toa,
//     const uint8_t fToA, const uint16_t spidrTime) {

//   uint64_t pixelClockTime = int(409600 * spidrTime + 25 * toa - 1.5625 * fToA);

//   if (lastEpochPulseTime.pairedTDCDataEvent.tdcTimeInPixelClock <
//       pixelClockTime) {
//     uint32_t timeUntilReset =
//         PIXEL_MAX_TIMESTAMP_NS -
//         lastEpochPulseTime.pairedTDCDataEvent.tdcTimeInPixelClock;

//     tof = make_unique<uint32_t>(timeUntilReset + pixelClockTime);

//     toaInEpochNs =
//       std::make_unique<uint64_t>(lastEpochPulseTime.pulseTimeInEpochNs +
//                      *tof);
//   } else {
//     tof = make_unique<uint32_t>(
//         pixelClockTime -
//         lastEpochPulseTime.pairedTDCDataEvent.tdcTimeInPixelClock);

//     toaInEpochNs = std::make_unique<uint64_t>(
//         lastEpochPulseTime.pulseTimeInEpochNs + *tof);
//   }

//   return *this;
// }

// // Implementation of build method for PixelDataEventBuilder
// PixelDataEvent *PixelDataEventBuilder::build() {
//   if (dCol == nullptr || sPix == nullptr || pix == nullptr || ToT == nullptr ||
//       tof == nullptr || toaInEpochNs == nullptr) {
//     throw std::runtime_error("PixelDataEventBuilder: Not all required fields set");
//   }

//   return new PixelDataEvent(
//       std::move(dCol), std::move(sPix), std::move(pix), std::move(ToT),
//       std::move(tof), std::move(toaInEpochNs));
// }

// Implementation of PixelDataEvent private constructor
PixelDataEvent::PixelDataEvent(uint16_t dCol, uint16_t sPix, uint8_t pix,
                 uint16_t ToT, uint8_t fToA, uint16_t toa, uint32_t spidrTime)
  : dCol(dCol), sPix(sPix), pix(pix), ToT(ToT), fToA(fToA), toa(toa), spidrTime(spidrTime) {}

} // namespace Timepix3