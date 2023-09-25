// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configuration and calibration from
/// file
///         superclass for detector specific classes to inherit from
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/VMM3Calibration.h>
#include <common/readout/vmm3/VMM3Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void VMM3Config::loadAndApplyConfig() {
  root = from_json_file(FileName);
  applyVMM3Config();
  applyConfig();
}

void VMM3Config::applyVMM3Config() {
  std::string Name;
  try {
    FileParameters.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (FileParameters.InstrumentName != ExpectedName) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name");
  }

  try {
    FileParameters.InstrumentGeometry =
        root["InstrumentGeometry"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for InstrumentGeometry");
  }
  LOG(INIT, Sev::Info, "InstrumentGeometry {}",
      FileParameters.InstrumentGeometry);

  try {
    FileParameters.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeNS {}", FileParameters.MaxPulseTimeNS);

  try {
    auto PanelConfig = root["Config"];
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString = Mapping["HybridId"];

      XTRACE(INIT, DEB, "Ring %d, FEN %d, Hybrid %d", Ring, FEN, LocalHybrid);

      if ((Ring > MaxRing) or (FEN > MaxFEN) or (LocalHybrid > MaxHybrid)) {
        auto Message = fmt::format("Invalid Ring/FEN/VMM config: {}/{}/{}", Ring, FEN, LocalHybrid);
        XTRACE(INIT, ERR, "%s", Message.c_str());
        throw std::runtime_error(Message);
      }

      // so far only check for correct length
      if (!validHybridId(IDString)) {
        auto Message = fmt::format("Invalid HybridId {}", IDString);
        XTRACE(INIT, ERR, "%s", Message.c_str());
        throw std::runtime_error(Message);
      }

      // has it been added before?
      if (lookupHybrid(IDString)) {
        auto Message = fmt::format("Duplicate HybridID {}", IDString);
        XTRACE(INIT, ERR, "%s", Message.c_str());
        throw std::runtime_error(Message);
      }

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);
      XTRACE(INIT, DEB, "Hybrid at: %p", &Hybrid);

      if (Hybrid.Initialised) {
        auto Message = fmt::format("Hybrid {} on Ring/FEN {}/{} is already initialised",
          LocalHybrid, Ring, FEN);
        XTRACE(INIT, ERR, "%s", Message.c_str());
        throw std::runtime_error(Message);
      }

      Hybrid.Initialised = true;
      Hybrid.HybridId = IDString;

      auto Message = fmt::format("JSON config - Detector {}, Hybrid {}, Ring {}, FEN {}, LocalHybrid "
      "{}", Name, NumHybrids, Ring, FEN, LocalHybrid);
      XTRACE(INIT, DEB, Message.c_str());
      LOG(INIT, Sev::Info, Message);

      Hybrid.HybridNumber = NumHybrids;
      NumHybrids++;
    }

    LOG(INIT, Sev::Info,
        "JSON config - Detector has {} cassettes/hybrids and {} pixels",
        NumHybrids, NumPixels);
  } catch (...) {
    auto Message = fmt::format("JSON config - error: Invalid Config file: {}",FileName);
    XTRACE(INIT, ERR, "%s", Message.c_str());
    LOG(INIT, Sev::Error, Message);
    throw std::runtime_error("Message");
  }
}

void VMM3Config::loadAndApplyCalibration(std::string CalibFile) {
  nlohmann::json calib_root;
  try {
    calib_root = from_json_file(CalibFile);
  } catch (...) {
    std::string Message = "Error loading json calibration file";
    LOG(INIT, Sev::Error, Message.c_str());
    throw std::runtime_error(Message);
  }

  std::string Name = calib_root["Detector"];
  unsigned Version = calib_root["Version"].get<unsigned int>();
  auto Calibrations = calib_root["Calibrations"];

  if (Name != ExpectedName) {
    throw std::runtime_error("Calibration file is for incorrect detector");
  }

  if (Version != 1) {
    throw std::runtime_error("Unsupported calibration file version");
  }

  for (auto &Calibration : Calibrations) {
    std::string HybridId = Calibration["VMMHybridCalibration"]["HybridId"];
    if (!validHybridId(HybridId)) {
      throw std::runtime_error("Invalid HybridID in Calibration file");
    }
    applyCalibration(HybridId, Calibration);
  }
}

/// \brief validate the hybrid id string
/// \todo too simplistic?
bool VMM3Config::validHybridId(std::string HybridID) {
  return (HybridID.length() == 32);
}

void VMM3Config::applyCalibration(std::string HybridID,
                                  nlohmann::json Calibration) {

  ESSReadout::Hybrid &CurrentHybrid = getHybrid(HybridID);

  std::string Date = Calibration["VMMHybridCalibration"]["CalibrationDate"];

  // Apply calibration below
  XTRACE(INIT, ALW, "Hybrid ID %s, Date %s", HybridID.c_str(), Date.c_str());

  auto &vmm0cal = Calibration["VMMHybridCalibration"]["vmm0"];
  applyVMM3Calibration(CurrentHybrid, 0, vmm0cal);
  auto &vmm1cal = Calibration["VMMHybridCalibration"]["vmm1"];
  applyVMM3Calibration(CurrentHybrid, 1, vmm1cal);
}

void VMM3Config::applyVMM3Calibration(ESSReadout::Hybrid &Hybrid,
                                      unsigned vmmid,
                                      nlohmann::json VMMCalibration) {

  auto adc_offset = VMMCalibration["adc_offset"];
  auto adc_slope = VMMCalibration["adc_slope"];
  auto tdc_offset = VMMCalibration["tdc_offset"];
  auto tdc_slope = VMMCalibration["tdc_slope"];

  unsigned TotalLength = adc_offset.size() + adc_slope.size() +
                         tdc_offset.size() + tdc_slope.size();

  if (TotalLength != 4 * VMM3Calibration::CHANNELS) {
    throw std::runtime_error("Wrong number of channels in calibration");
  }
  for (unsigned Channel = 0; Channel < VMM3Calibration::CHANNELS; Channel++) {
    Hybrid.VMMs[vmmid].setCalibration(Channel, tdc_offset[Channel],
                                      tdc_slope[Channel], adc_offset[Channel],
                                      adc_slope[Channel]);
    XTRACE(INIT, DEB, "Hybrid at: %p", &Hybrid);
    XTRACE(INIT, DEB,
           "Setting Calibration for Channel %u, tdc_offset %f, tdc_slope %f, "
           "adc_offset %f, adc_slope %f",
           Channel, (double)tdc_offset[Channel], (double)tdc_slope[Channel],
           (double)adc_offset[Channel], (double)adc_slope[Channel]);
  }
}
