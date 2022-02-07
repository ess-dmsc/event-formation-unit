// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/VMM3Calibration.h>
#include <common/readout/vmm3/VMM3Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void VMM3Config::loadAndApplyConfig() {
  root = from_json_file(FileName);
  applyConfig();
}

void VMM3Config::loadAndApplyCalibration(std::string CalibFile) {
  nlohmann::json calib_root;
  try {
    calib_root = from_json_file(CalibFile);
  } catch (...) {
    LOG(INIT, Sev::Error, "Error loading json calibration file");
    throw std::runtime_error("Error loading json calibration file");
  }

  std::string Name = calib_root["Detector"];
  unsigned Version = calib_root["Version"].get<unsigned int>();
  // unsigned NumHybrids = root["Hybrids"].get<unsigned int>();
  auto Calibrations = calib_root["Calibrations"];

  if (Name != ExpectedName) {
    throw std::runtime_error("Calibration file is for incorrect detector");
  }

  if (Version != 1) {
    throw std::runtime_error("Unsupported calibration file version");
  }

  for (auto &Calibration : Calibrations) {
    std::string HybridId = Calibration["VMMHybridCalibration"]["HybridId"];
    applyCalibration(HybridId, Calibration);
  }
}

void VMM3Config::applyVMM3Calibration(ESSReadout::Hybrid &Hybrid,
                                      unsigned vmmid,
                                      nlohmann::json VMMCalibration) {

  auto adc_offset = VMMCalibration["adc_offset"];
  auto adc_slope = VMMCalibration["adc_slope"];
  auto tdc_offset = VMMCalibration["tdc_offset"];
  auto tdc_slope = VMMCalibration["tdc_slope"];

  unsigned TotalLength = adc_offset.size() + adc_slope.size() +
                         adc_offset.size() + adc_slope.size();

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
