/** Copyright (C) 2016-2018 European Spallation Source */

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
