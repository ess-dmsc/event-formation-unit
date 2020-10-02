// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie Readouts builder
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/Buffer.h>
#include <jalousie/Readout.h>
#include <vector>

namespace Jalousie {

class BuilderReadouts {
public:
  void parse(Buffer<uint8_t> buffer);
  std::string debug() const;

  std::vector<Readout> parsed_data;

protected:
  // preallocated
  Readout readout_;
};

}
