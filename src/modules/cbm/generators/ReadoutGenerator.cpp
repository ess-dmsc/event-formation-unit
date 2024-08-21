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

#include <chrono>
#include <common/debug/Trace.h>
#include <common/time/ESSTime.h>
#include <functional>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/LinearGenerator.h>
#include <modules/cbm/generators/ReadoutGenerator.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

ReadoutGenerator::ReadoutGenerator()
    : ReadoutGeneratorBase(ESSReadout::Parser::DetectorType::CBM) {

  // Set default values for the generator
  app.add_option("--monitor_type", cbmSettings.monitorType,
                 "Beam monitor type (TTL, N2GEM, IBM, etc)");
  app.add_option("--fen", cbmSettings.FenId, "Override FEN ID (default 0)");
  app.add_option("--channel", cbmSettings.ChannelId,
                 "Override channel ID (default 0)");
  app.add_option(
      "--value", cbmSettings.genValue,
      "Fixed value for value generator (required for Fixed generator type)");

  auto checkGenType = [this](std::string &value) -> std::string {
    if (value == "Fixed" && !cbmSettings.genValue.has_value()) {
      throw CLI::ValidationError("Requires --value to be set");
    } else if (value == "Linear" && !cbmSettings.genValue.has_value()) {
      throw CLI::ValidationError("Requires --value to be set");
    }
    return "";
  };

  CLI::Validator genTypeValidator =
      CLI::Validator(checkGenType, "Invalid value");

  app.add_option("--generator_type", cbmSettings.generatorTypeStr,
                 "Type of generator (Distribution, Linear, Fixed)")
      ->check(CLI::IsMember({"Distribution", "Linear", "Fixed"}))
      ->check(genTypeValidator)
      ->transform([this](const std::string &str) {
        if (str == "Distribution") {
          cbmSettings.generatorType = GeneratorType::Distribution;
        } else if (str == "Linear") {
          cbmSettings.generatorType = GeneratorType::Linear;
        } else if (str == "Fixed") {
          cbmSettings.generatorType = GeneratorType::Fixed;
        } else {
          std::cout << "Invalid generator type\n";
          std::exit(-1);
        }
        return str;
      });
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

    if (cbmSettings.generatorType == GeneratorType::Distribution) {
      distributionValueGenerator(dataPkt);
    } else if (cbmSettings.generatorType == GeneratorType::Linear) {
      linearValueGenerator(dataPkt);
    } else {
      fixedValueGenerator(dataPkt);
    }

    // Increment time for next readout
    addTicksBtwReadoutsToReadoutTime();

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

void ReadoutGenerator::distributionValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {
    Generator =
        std::make_unique<DistributionGenerator>(MILLISEC / Settings.Frequency);
  }

  esstime::TimeDurationNano Tof = getReadoutTimeNs() - getPulseTimeNs();
  value->NPos =
      1000 * Generator->getValue(static_cast<double>(Tof.count() / 1000000.0));
}

void ReadoutGenerator::linearValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {
    Generator = std::make_unique<LinearGenerator>(cbmSettings.genValue.value());
  }

  esstime::TimeDurationNano Tof = getReadoutTimeNs() - getPulseTimeNs();
  value->NPos =
      Generator->getValue(static_cast<double>(Tof.count() / 1000000.0));
}

void ReadoutGenerator::fixedValueGenerator(Parser::CbmReadout *value) {
  value->NPos = cbmSettings.genValue.value();
}

} // namespace cbm
// GCOVR_EXCL_STOP
