/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibraiton files for VMM asics
///
//===----------------------------------------------------------------------===//

#include <common/Trace.h>
#include <gdgem/vmm3/CalibrationFile.h>
#include <stdio.h>
#include <string.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// \brief clear the calibration array
CalibrationFile::CalibrationFile() {
  auto n = sizeof(calibrations)/sizeof(calibration_t);
  for (size_t i = 0; i < n; i++) {
    ((struct calibration_t *)calibrations)[i] = nocorr;
  }
}

int CalibrationFile::addCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo, float offset, float slope) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId, vmmId, chNo);
    return -1;
  }
  calibrations[fecId][vmmId][chNo].slope = slope;
  calibrations[fecId][vmmId][chNo].offset = offset;
  return 0;
}

struct CalibrationFile::calibration_t & CalibrationFile::getCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo) {
  if ((fecId >= MAX_FEC) or (vmmId >= MAX_VMM) or (chNo >= MAX_CH)) {
    XTRACE(INIT, DEB, "invalid offsets: fec: %d, vmm: %d, ch:%d\n", fecId, vmmId, chNo);
    return errcorr;
  }

  return calibrations[fecId][vmmId][chNo];
}
