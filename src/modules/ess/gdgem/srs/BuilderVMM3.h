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
#include <algorithm>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Gem {

struct NMXStats;

class BuilderVMM3 : public AbstractBuilder {
public:
  BuilderVMM3(SRSTime time_intepreter, SRSMappings  digital_geometry,
              uint16_t adc_threshold, std::string dump_dir,
              unsigned int pmin,
              unsigned int pmax,
              unsigned int pwidth,
              std::shared_ptr<CalibrationFile> calfile, NMXStats & stats, bool enable_data_processing);

  ~BuilderVMM3() { XTRACE(INIT, DEB, "BuilderVMM3 destructor called"); }

  /// \todo Martin document
  void process_buffer(char *buf, size_t size) override;

 private:
  std::shared_ptr<CalibrationFile> calfile_;

  SRSTime time_intepreter_;
  SRSMappings digital_geometry_;
  uint16_t adc_threshold_ {0};
  unsigned int PMin, PMax, PWidth; ///< REMOVE eventually - experimental
  NMXStats & stats_;
  ParserVMM3 parser_;
  std::shared_ptr<ReadoutFile> readout_file_;
  bool data_processing_ {true};

  // preallocated and reused
  Readout readout;
  Hit hit;
};

}
