// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial readout data for CBM beam monitor types
///        based on TTLMon ICD
/// \todo add link
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include "generators/functiongenerators/DistributionGenerator.h"
#include <chrono>
#include <common/debug/Trace.h>
#include <common/time/ESSTime.h>
#include <modules/cbm/generators/ReadoutGenerator.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

ReadoutGenerator::ReadoutGenerator()
    : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::CBM) {
  app.add_option("--monitor_type", cbmSettings.monitorType,
                 "Beam monitor type (TTL, N2GEM, IBM, etc)");
  app.add_option("--fen", cbmSettings.FenId, "Override FEN ID (default 0)");
  app.add_option("--channel", cbmSettings.ChannelId,
                 "Override channel ID (default 0)");
}

void ReadoutGenerator::generateData() {
  if (FunctionGenerator == nullptr) {
    FunctionGenerator =
        std::make_unique<DistributionGenerator>(MILLISEC / Settings.Frequency);
  }

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  if (cbmSettings.monitorType == CbmType::TTL) {
    generateTTLData(dataPtr);
  } else if (cbmSettings.monitorType == CbmType::IBM) {
    generateIBMData(dataPtr);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }
}

// Generate data for TTL monitor
void ReadoutGenerator::generateTTLData(uint8_t *dataPtr) {

  for (uint32_t Readout = 0; Readout < numberOfReadouts; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    dataPkt->TimeHigh = getReadoutTimeHigh();
    dataPkt->TimeLow = getReadoutTimeLow();
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Increment time for next readout and adjust high time if needed
    addTicksBtwReadoutsToReadoutTime();

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for IBM type beam monitors
void ReadoutGenerator::generateIBMData(uint8_t *dataPtr) {

  esstime::TimeDurationNano nextPulseTime = getNextPulseTimeNs();

  for (uint32_t Readout = 0; Readout <= numberOfReadouts; Readout++) {

    // Check if we need to generate new pulse time and reset readout time
    // stop generating readouts and sync readout time with new spulse time
    if (getReadoutTimeNs() > nextPulseTime) {
      resetReadoutToPulseTime();
      break;
    }

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->TimeHigh = getReadoutTimeHigh();
    dataPkt->TimeLow = getReadoutTimeLow();
    dataPkt->Type = CbmType::IBM;

    // Currently we generating for 1 beam monitor only
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 0;

    esstime::TimeDurationNano Tof = getReadoutTimeNs() - getPulseTimeNs();
    dataPkt->NPos = 1000 * FunctionGenerator->getDistFromTof(
                               static_cast<double>(Tof.count() / 1000000.0));

    // Increment time for next readout
    addTicksBtwReadoutsToReadoutTime();

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

} // namespace cbm
// GCOVR_EXCL_STOP
