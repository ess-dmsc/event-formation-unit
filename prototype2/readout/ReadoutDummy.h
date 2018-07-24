/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple example of a derived class of the Readout base class
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <readout/Readout.h>

class ReadoutDummy : public Readout {
public:
  ReadoutDummy(unsigned int type) : detectortype(type){};

  int parse(const char *buffer, uint32_t size);

private:
  unsigned int detectortype{0};
};
