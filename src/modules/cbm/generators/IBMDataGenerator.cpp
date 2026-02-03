// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of IBM data generator
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <generators/functiongenerators/FunctionGenerator.h>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/generators/IBMDataGenerator.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

IBMDataGenerator::IBMDataGenerator(
    uint8_t fiberId, uint8_t fenId, uint8_t channelId, uint32_t numReadouts,
    uint8_t normFactor, bool shakeBeam, bool randomise, double magFactor,
    std::function<std::pair<uint32_t, uint32_t>()> readoutTimeGenerator,
    std::unique_ptr<FunctionGenerator> valueGenerator)
    : FiberId(fiberId), FenId(fenId), ChannelId(channelId),
      NumReadouts(numReadouts), NormFactor(normFactor), ShakeBeam(shakeBeam),
      Randomise(randomise), MagFactor(magFactor),
      ReadoutTimeGenerator(readoutTimeGenerator),
      ValueGenerator(std::move(valueGenerator)) {}

void IBMDataGenerator::generateData(uint8_t *dataPtr,
                                    uint32_t readoutsPerPacket,
                                    esstime::ESSTime pulseTime) const {

  // Generate new random drift for each new pulse when ShakeBeam is enabled
  if (ShakeBeam && pulseTime != LastPulseTime) {
    LastPulseTime = pulseTime;

    // Convert microseconds to milliseconds for the drift
    PulseDriftMs =
        esstime::usToMilliseconds((BeamShakeDistUs(RandomGenerator))).count();

    // Reset counter at the start of each pulse
    NumberOfReadoutsGenerated = 0;
  }

  for (uint32_t readout = 0; readout < readoutsPerPacket; readout++) {

    if (NumberOfReadoutsGenerated < NumReadouts) {

      // Get pointer to the data buffer and clear memory with zeros
      auto dataPkt = (Parser::CbmReadout *)dataPtr;
      memset(dataPkt, 0, sizeof(Parser::CbmReadout));

      // write data packet to the buffer
      dataPkt->FiberId = FiberId;
      dataPkt->FENId = FenId;
      dataPkt->DataLength = sizeof(Parser::CbmReadout);
      auto [readoutTimeHigh, readoutTimeLow] = ReadoutTimeGenerator();
      dataPkt->TimeHigh = readoutTimeHigh;
      dataPkt->TimeLow = readoutTimeLow;
      dataPkt->Type = CbmType::IBM;

      // Currently we generating for 1 beam monitor only
      dataPkt->Channel = ChannelId;
      dataPkt->ADC = 0;

      auto readoutTime = esstime::ESSTime(dataPkt->TimeHigh, dataPkt->TimeLow);
      auto TofMs = esstime::nsToMilliseconds(readoutTime - pulseTime).count();

      // Apply pulse-constant random drift for beam shaking
      if (ShakeBeam && TofMs > PulseDriftMs) {
        TofMs -= PulseDriftMs;
      }

      int Noise{0};

      // Add noise to the value if enabled
      if (Randomise) {
        Noise = NoiseDist(RandomGenerator);
      }

      uint32_t value = static_cast<uint32_t>(
                           MagFactor * ValueGenerator->getValueByPos(TofMs)) +
                       Noise;
      dataPkt->NADC.setNADC(value);
      dataPkt->NADC.MCASum = NormFactor;

      // Move pointer to next readout
      dataPtr += sizeof(Parser::CbmReadout);
      NumberOfReadoutsGenerated++;
    }
  }
}

} // namespace cbm
// GCOVR_EXCL_STOP
