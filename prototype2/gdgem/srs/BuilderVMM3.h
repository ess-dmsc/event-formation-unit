/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for creating NMX hits from SRS/VMM data
///
//===----------------------------------------------------------------------===//

#pragma once
#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>
#include <gdgem/srs/ParserVMM3.h>
#include <gdgem/srs/CalibrationFile.h>
#include <gdgem/nmx/Readout.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Gem {

class BuilderVMM3 : public AbstractBuilder {
public:
  BuilderVMM3(SRSTime time_intepreter, SRSMappings digital_geometry,
              uint16_t adc_threshold, std::string dump_dir,
              std::shared_ptr<CalibrationFile> calfile);

  ~BuilderVMM3() { XTRACE(INIT, DEB, "BuilderVMM3 destructor called"); }

  /// \todo Martin document
  void process_buffer(char *buf, size_t size) override;

 private:
  std::shared_ptr<CalibrationFile> calfile_;
  VMM3SRSData parser_;
  SRSTime time_intepreter_;
  SRSMappings digital_geometry_;

  uint16_t adc_threshold_ {0};

  std::shared_ptr<ReadoutFile> readout_file_;

  // preallocated and reused
  Readout readout;
  Hit hit;
};

}
