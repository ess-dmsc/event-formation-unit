// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of EVENT_0D data generator
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/generators/Event0DDataGenerator.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

void Event0DDataGenerator::generateData(uint8_t *dataPtr,
                                       uint32_t readoutsPerPacket,
                                       esstime::ESSTime pulseTime) const {
  (void)pulseTime;  // Unused - Event0D doesn't use pulse time
  for (uint32_t readout = 0; readout < readoutsPerPacket; readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = FiberId;
    dataPkt->FENId = FenId;
    auto [readoutTimeHigh, readoutTimeLow] = ReadoutTimeGenerator();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = CbmType::EVENT_0D;
    dataPkt->Channel = ChannelId;
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

} // namespace cbm
// GCOVR_EXCL_STOP
