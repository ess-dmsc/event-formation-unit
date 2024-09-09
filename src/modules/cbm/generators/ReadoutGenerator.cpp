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
#include <common/utils/EfuUtils.h>
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

  CLI::Validator genTypeValidator = CLI::Validator(
      [this](std::string &value) -> std::string {
        if (value == "Fixed" && !cbmSettings.Value.has_value()) {
          throw CLI::ValidationError("Requires --value to be set");
        } else if (value == "Linear" && !cbmSettings.Gradient.has_value()) {
          throw CLI::ValidationError("Requires --gradient to be set");
        }
        return "";
      },
      "");

  auto CbmGroup = app.add_option_group("CBM Options");
  CbmGroup->add_option("--monitor_type", cbmSettings.monitorType,
                       "Beam monitor type (TTL, N2GEM, IBM, etc)");
  CbmGroup->add_option("--fen", cbmSettings.FenId,
                       "Override FEN ID (default 0)");
  CbmGroup->add_option("--channel", cbmSettings.ChannelId,
                       "Override channel ID (default 0)");

  auto IbmGroup = app.add_option_group("IBM Options");

  IbmGroup->add_option(
      "--value", cbmSettings.Value,
      "Fixed value for the value function (required for Fixed generator type)");
  IbmGroup->add_option(
      "--gradient", cbmSettings.Gradient,
      "Gradient of the Linear function (required for Linear generator type)");
  IbmGroup->add_option("--offset", cbmSettings.Offset,
                       "Function generator offset for the start value "
                       "(Optional for all generator type)");
  IbmGroup->add_option("--bins", cbmSettings.NumberOfBins,
                       "Number of bins (sampling) of the distribution function (default 512)");

  std::string genTypeStr = "";

  IbmGroup
      ->add_set("--generator_type", genTypeStr, {"Dist", "Linear", "Fixed"},
                "Set the generator type (default : Dist)")
      ->check(genTypeValidator)
      ->transform([this](const std::string &str) {
        if (str == "Dist") {
          cbmSettings.generatorType = GeneratorType::Distribution;
        } else if (str == "Linear") {
          cbmSettings.generatorType = GeneratorType::Linear;
        } else if (str == "Fixed") {
          cbmSettings.generatorType = GeneratorType::Fixed;
        } else {
          throw CLI::ValidationError("Invalid generator type: " + str);
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
  value->NPos = 1000 * Generator->getValue(
                           efutils::nsToMilliseconds(Tof.count()).count());
}

void ReadoutGenerator::linearValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {
    Generator = std::make_unique<LinearGenerator>(MILLISEC / Settings.Frequency,
                                                  cbmSettings.Gradient.value(),
                                                  cbmSettings.Offset);
  }

  esstime::TimeDurationNano Tof = getReadoutTimeNs() - getPulseTimeNs();
  value->NPos =
      Generator->getValue(efutils::nsToMilliseconds(Tof.count()).count());
}

void ReadoutGenerator::fixedValueGenerator(Parser::CbmReadout *value) {
  value->NPos = cbmSettings.Value.value() + cbmSettings.Offset;
}

} // namespace cbm
// GCOVR_EXCL_STOP
