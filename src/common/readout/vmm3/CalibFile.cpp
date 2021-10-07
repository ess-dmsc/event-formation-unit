// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get Hybrid/VMM3 calibration data from json
///
//===----------------------------------------------------------------------===//


#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/CalibFile.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace ESSReadout {

  void CalibFile::load(std::string CalibFile) {
    LOG(INIT, Sev::Info, "Loading calibration file {}", CalibFile.c_str());

    try {
      root = from_json_file(CalibFile);
    } catch (...) {
      LOG(INIT, Sev::Error, "Error loading json calibration file");
      throw std::runtime_error("Error loading json calibration file");
    }

    std::string Name = root["Detector"];
    unsigned Version = root["Version"].get<unsigned int>();
    unsigned NumHybrids = root["Hybrids"].get<unsigned int>();
    auto Calibrations = root["Calibrations"];

    if (Name != Detector) {
      throw std::runtime_error("Calibration file is not for Freia");
    }

    if (Version != 1) {
      throw std::runtime_error("Unsupported calibration file version");
    }

    if (NumHybrids != Hybrids.size()) {
      throw std::runtime_error("Calibration size mismatch");
    }

    unsigned i = 0;
    for (auto & Calibration : Calibrations) {
      unsigned index = Calibration["VMMHybridCalibration"]["HybridIndex"].get<unsigned int>();
      if (i != index) {
        throw std::runtime_error("Hybrid enumeration error");
      }
      applyCalibration(index, Calibration);
      i++;
    }

  }


  void CalibFile::applyVMM3Calibration(Hybrid & Hybrid, unsigned vmmid,
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
    for (unsigned Channel = 0; Channel < 64; Channel++) {
      Hybrid.VMMs[vmmid].setCalibration(Channel,
        tdc_offset[Channel], tdc_slope[Channel],
        adc_offset[Channel], adc_slope[Channel]);
    }
  }


  void CalibFile::applyCalibration(unsigned HybridIndex,
    nlohmann::json Calibration) {

    std::string NewId = Calibration["VMMHybridCalibration"]["HybridId"];
    if (NewId.size() != Hybrid::IdSize) {
      throw std::runtime_error("HybridId string has wrong length");
    }

    if (not Hybrid::isAvailable(NewId, Hybrids)) {
      throw std::runtime_error("Duplicate HybridId in calibration");
    }

    std::string Date = Calibration["VMMHybridCalibration"]["CalibrationDate"];

    // Apply calibration below
    XTRACE(INIT, ALW, "Hybrid %3u, ID %s, Date %s",
           HybridIndex, NewId.c_str(), Date.c_str());

    Hybrids[HybridIndex].HybridId = NewId;

    auto & vmm0cal = Calibration["VMMHybridCalibration"]["vmm0"];
    applyVMM3Calibration(Hybrids[HybridIndex], CalibFile::VMM0, vmm0cal);
    auto & vmm1cal = Calibration["VMMHybridCalibration"]["vmm1"];
    applyVMM3Calibration(Hybrids[HybridIndex], CalibFile::VMM1, vmm1cal);
  }

} // namespace ESSReadout
