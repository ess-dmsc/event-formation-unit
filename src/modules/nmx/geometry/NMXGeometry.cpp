// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX geometry implementation
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <nmx/geometry/NMXGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

using namespace vmm3;

namespace Nmx {

bool NMXGeometry::validateReadoutData(const VMM3Parser::VMM3Data &Data) {
  uint8_t Ring = calcRing(Data.FiberId);
  uint8_t HybridId = calcHybridId(Data.VMM);

  return validateAll(
      [&]() { return validateRing(Ring); },
      [&]() { return validateFEN(Data.FENId); },
      [&]() { return validateHybrid(Ring, Data.FENId, HybridId); },
      [&]() {
        return validateADC(getRawADC(Data.OTADC), Ring, Data.FENId, HybridId);
      },
      [&]() {
        return validateAsicIdAndChannel(getAsicId(Data.VMM), Data.Channel);
      });
}

bool NMXGeometry::validateADC(uint16_t ADC, int Ring, uint8_t FENId,
                              uint8_t HybridId) const {
  // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
  // uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);
  // no calibration yet, so using raw ADC value

  // Now we add readouts with the calibrated time and adc to the panel
  // builders
  const auto &Hybrid = Conf.getHybrid(Ring, FENId, HybridId);

  if (ADC < Hybrid.MinADC) {
    XTRACE(DATA, INF, "ADC lower than minimum, ADC: %u, minimum ADC: %u", ADC,
           Hybrid.MinADC);
    NmxCounters.ADCErrors++;
    return false;
  }
  return true;
}

uint16_t NMXGeometry::coord(uint8_t Channel, uint8_t AsicId, uint16_t Offset,
                            bool ReversedChannels) {
  // NMX use case: During event reduction, we need to calculate coordinates
  // from clustered hits. At this point, we already have the AsicId and
  // Channel, but not the original VMM. The AsicId comes from prior
  // validation during readout processing where VMM was available.

  if (!validateAsicIdAndChannel(AsicId, Channel)) {
    Counters.CoordOverflow++;
    return InvalidCoord;
  }

  uint16_t CoordinateValue;

  if (ReversedChannels) {
    // Channel order is inverted for this half of the panel
    CoordinateValue =
        Offset + ((1 - AsicId) * NumStrips) + (NumStrips - 1 - Channel);
  } else {
    // Normal channel order
    CoordinateValue = Offset + (AsicId * NumStrips) + Channel;
  }

  XTRACE(DATA, DEB,
         "Channel %d, AsicId %d, Offset %d, Reversed %d -> Coordinate %u",
         Channel, AsicId, Offset, ReversedChannels, CoordinateValue);

  return CoordinateValue;
}

} // namespace Nmx