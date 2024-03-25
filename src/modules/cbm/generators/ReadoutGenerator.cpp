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

#include <common/debug/Trace.h>
#include <cstdint>
#include <cstring>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <modules/cbm/generators/ReadoutGenerator.h>
#include <modules/cbm/geometry/Parser.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

ReadoutGenerator::ReadoutGenerator() : ReadoutGeneratorBase() {
  app.add_option("--monitor_type", cbmSettings.monitorType,
                 "Beam monitor type (TTL, N2GEM, IBM, etc)");
}

void ReadoutGenerator::generateData() {
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
  uint32_t dataTimeHigh = PulseTimeHigh;
  uint32_t dataTimeLow = PulseTimeLow;

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = CBM_FEN_ID;
    dataPkt->TimeHigh = dataTimeHigh;
    dataPkt->TimeLow = dataTimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = (Readout % 3) & 0x01; // 0 0 1, 0 0 1, ...
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Increment time for next readout and adjust high time if needed
    dataTimeLow += Settings.TicksBtwReadouts;
    if (dataTimeLow >= 88052499) {
      dataTimeLow -= 88052499;
      dataTimeHigh += 1;
    }

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for IBM type beam monitors
void ReadoutGenerator::generateIBMData(uint8_t *dataPtr) {

  uint32_t dataTimeHigh = PulseTimeHigh;
  uint32_t dataTimeLow = PulseTimeLow;

  uint32_t dataValue = 100000;

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
        
    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = CBM_FEN_ID;
    dataPkt->TimeHigh = dataTimeHigh;
    dataPkt->TimeLow = dataTimeLow;
    dataPkt->Type = CbmType::IBM;

    // Currently we generating for 1 beam monitor only
    dataPkt->Channel = 0;
    dataPkt->ADC = 0;
    dataPkt->NPos = Fuzzer.randomInterval(1, 1000) + dataValue;

    if (Readout > Settings.NumReadouts / 4 &&
        Readout < 3 * Settings.NumReadouts / 4) {
      dataValue += 10000;
    } else {
      dataValue = 0;
    }

    // Increment time for next readout and adjust high time if needed
    dataTimeLow += Settings.TicksBtwReadouts;
    if (dataTimeLow >= 88052499) {
      dataTimeLow -= 88052499;
      dataTimeHigh += 1;
    }

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}
} // namespace cbm
// GCOVR_EXCL_STOP
