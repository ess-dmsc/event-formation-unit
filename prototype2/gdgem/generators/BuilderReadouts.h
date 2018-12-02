/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for creating NMX hits from h5 data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/nmx/Readout.h>
#include <common/clustering/Hit.h>
#include <gdgem/srs/SRSMappings.h>

namespace Gem {

class BuilderReadouts : public AbstractBuilder {
public:
  BuilderReadouts(SRSMappings digital_geometry, uint16_t adc_threshold, std::string dump_dir);

  /// \todo Martin document
  // \todo use Buffer<char>
  void process_buffer(char *buf, size_t size) override;

private:
  // from params
  std::shared_ptr<HitFile> hit_file_; //dumpfile
  SRSMappings digital_geometry_;
  uint16_t adc_threshold_ {0};

  // preallocated
  std::vector<Readout> converted_data;
  Hit hit;
};

}