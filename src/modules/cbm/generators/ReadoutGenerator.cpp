// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial TTLMon readout
/// based on TTLMon ICD
/// \todo add link
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <common/debug/Trace.h>
#include <cstdint>
#include <cstring>
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

  uint32_t TimeHigh = 0;
  uint32_t TimeLow = 0;

  if (Settings.headerVersion == ESSReadout::Parser::HeaderVersion::V0) {
    TimeHigh = reinterpret_cast<ESSReadout::Parser::PacketHeaderV0 *>(&Buffer)
                   ->PulseHigh;
    TimeLow = reinterpret_cast<ESSReadout::Parser::PacketHeaderV0 *>(&Buffer)
                  ->PulseLow;
  } else if (Settings.headerVersion == ESSReadout::Parser::HeaderVersion::V1) {
    TimeHigh = reinterpret_cast<ESSReadout::Parser::PacketHeaderV1 *>(&Buffer)
                   ->PulseHigh;
    TimeLow = reinterpret_cast<ESSReadout::Parser::PacketHeaderV1 *>(&Buffer)
                  ->PulseLow;
  } else {
    throw std::runtime_error("Incorrect header version");
  }

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    // Cbm readout data length is always 20 bytes
    auto dataSize = sizeof(Parser::CbmReadout);
    assert(dataSize == 20);

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, dataSize);

    dataPkt->DataLength = dataSize;
    // Monitor is (so far) always on logical fiber 22 (ring 11), fen 0
    dataPkt->FiberId = 22;
    // Currently we only have one BM on FEN 0
    dataPkt->FENId = 0;
    dataPkt->TimeHigh = TimeHigh;
    dataPkt->TimeLow = TimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = (Readout % 3) & 0x01; // 0 0 1, 0 0 1, ...
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Increment time for next readout and adjust high time if needed
    TimeLow += Settings.TicksBtwEvents;
    if (TimeLow >= 88052499) {
      TimeLow -= 88052499;
      TimeHigh += 1;
    }

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}
} // namespace cbm
// GCOVR_EXCL_STOP
