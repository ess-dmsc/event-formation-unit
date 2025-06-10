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
    : ReadoutGeneratorBase(DetectorType::CBM) {

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

  // clang-format off

  auto CbmGroup = app.add_option_group("CBM Options");
  CbmGroup->add_option("--monitor_type", cbmSettings.monitorType,
                       "Beam monitor type (TTL, N2GEM, IBM, etc)");
  CbmGroup->add_option("--fen", cbmSettings.FenId,
                       "Override FEN ID (default 0)");
  CbmGroup->add_option("--channel", cbmSettings.ChannelId,
                       "Override channel ID (default 0)");

  auto IbmGroup = app.add_option_group("IBM Options");
  IbmGroup->add_option("--value", cbmSettings.Value,
      "Fixed value for the value function (required for Fixed generator type)");
  IbmGroup->add_option("--gradient", cbmSettings.Gradient,
      "Gradient of the Linear function (required for Linear generator type)");
  IbmGroup->add_option("--offset", cbmSettings.Offset,
                       "Function generator offset for the start value "
                       "(Optional for all generator type)");
  IbmGroup->add_option("--bins", cbmSettings.NumberOfBins,
      "Number of bins (sampling) of the distribution function (default 512)");

  IbmGroup->add_flag("--shake", cbmSettings.ShakeBeam,
                     "Use random drift value for each pulse to shake the beam "
                     "(default false)");
  IbmGroup->add_flag("--noise", cbmSettings.Randomise,
                     "Add noise to the distribution value (default false)");

  IbmGroup
      ->add_option("--generator_type", cbmSettings.generatorType,
                   "Set the generator type (default : Dist)")
      ->check(genTypeValidator);
}
// clang-format on

void ReadoutGenerator::generateData() {

  auto dataPtr = (uint8_t *)Buffer;
  dataPtr += HeaderSize;

  if (cbmSettings.monitorType == CbmType::EVENT_0D) {
    generateEvent0DData(dataPtr);
  } else if (cbmSettings.monitorType == CbmType::IBM) {
    generateIBMData(dataPtr);
  } else {
    throw std::runtime_error("Unsupported monitor type");
  }
}

// Generate data for 0D event monitor
void ReadoutGenerator::generateEvent0DData(uint8_t *dataPtr) {

  for (uint32_t Readout = 0; Readout < ReadoutPerPacket; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
    dataPkt->Type = cbmSettings.monitorType;
    dataPkt->Channel = cbmSettings.ChannelId;
    dataPkt->ADC = 12345;
    dataPkt->NPos = 0;

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

// Generate data for IBM type beam monitors
void ReadoutGenerator::generateIBMData(uint8_t *dataPtr) {

  if (cbmSettings.ShakeBeam) {
    // Use the pre-initialized RandomGenerator and ShakeBeamDist to generate
    // a random drift value for the whole pulse, which will be applied by
    // the function generator.
    RandomTimeDriftNS =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::microseconds(BeamShakeDistMs(RandomGenerator)));
  }

  for (uint32_t Readout = 0; Readout < ReadoutPerPacket; Readout++) {

    // Get pointer to the data buffer and clear memory with zeros
    auto dataPkt = (Parser::CbmReadout *)dataPtr;
    memset(dataPkt, 0, sizeof(Parser::CbmReadout));

    // write data packet to the buffer
    dataPkt->FiberId = CBM_FIBER_ID;
    dataPkt->FENId = cbmSettings.FenId;
    dataPkt->DataLength = sizeof(Parser::CbmReadout);
    auto [readoutTimeHigh, readoutTimeLow] = generateReadoutTime();
    dataPkt->TimeHigh = readoutTimeHigh;
    dataPkt->TimeLow = readoutTimeLow;
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

    // Move pointer to next readout
    dataPtr += sizeof(Parser::CbmReadout);
  }
}

void ReadoutGenerator::distributionValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {

    auto GenMaX = MILLISEC / Settings.Frequency;

    if (cbmSettings.ShakeBeam) {
      // Add the maximum shake beam time to the generator max value to ensure
      // that the generator can generate values for the whole shake beam range.
      // Generator max value is in milliseconds, shake beam range is in
      // microseconds.
      GenMaX += round(static_cast<float>(SHAKE_BEAM_US.second) / 1e3);
    }

    Generator = std::make_unique<DistributionGenerator>(
        static_cast<double>(GenMaX), cbmSettings.NumberOfBins);
  }

  esstime::TimeDurationMilli Tof =
      esstime::nsToMilliseconds(RandomTimeDriftNS) +
      esstime::TimeDurationMilli(Generator->getValue());

  int Noise{0};

  // Add noise to the distribution value if enabled
  if (cbmSettings.Randomise) {
    Noise = NoiseDist(RandomGenerator);
  }

  value->NPos = 1000 * Generator->getValueByIndex(Tof.count()) + Noise;
}

void ReadoutGenerator::linearValueGenerator(Parser::CbmReadout *value) {
  if (Generator == nullptr) {
    Generator = std::make_unique<LinearGenerator>(
      static_cast<double>(LinearGenerator::TimeDurationUnit / Settings.Frequency),
      static_cast<double>(cbmSettings.Gradient.value()),
      cbmSettings.Offset);
  }

  auto Tof = esstime::nsToMilliseconds(Generator->getValue());
  value->NPos = Generator->getValueByIndex(Tof.count());
}

void ReadoutGenerator::fixedValueGenerator(Parser::CbmReadout *value) {
  value->NPos = cbmSettings.Value.value() + cbmSettings.Offset;
}

} // namespace cbm
// GCOVR_EXCL_STOP
