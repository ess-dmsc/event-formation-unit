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
#include <gdgem/vmm3/ParserVMM3.h>
#include <gdgem/nmx/ReadoutFile.h>

#include <gdgem/clustering/HitSorter.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class BuilderVMM3 : public AbstractBuilder {
public:
  BuilderVMM3(SRSTime time_intepreter, SRSMappings geometry_interpreter,
              std::shared_ptr<AbstractClusterer> x, std::shared_ptr<AbstractClusterer> y,
              uint16_t adc_threshold_x, double max_time_gap_x,
              uint16_t adc_threshold_y, double max_time_gap_y,
              std::string dump_dir, bool dump_csv, bool dump_h5);

  ~BuilderVMM3() { XTRACE(INIT, DEB, "BuilderVMM2 destructor called\n"); }

  /// \todo Martin document
  ResultStats process_buffer(char *buf, size_t size) override;

private:
  VMM3SRSData parser_;
  SRSTime time_intepreter_;
  SRSMappings geometry_interpreter_;

  HitSorter sorter_x, sorter_y;

  std::shared_ptr<ReadoutFile> readout_file_;

  // preallocated and reused
  Readout readout;
  uint8_t plane;
  uint32_t geom_errors;
};
