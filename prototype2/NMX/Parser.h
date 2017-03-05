/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once

#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <NMX/Eventlet.h>
#include <vector>

class Parser {
public:
  /** @todo Martin document */
  std::vector<Eventlet> parse(char *buf, size_t size);

  std::vector<Eventlet> parse(unsigned int planeid, uint32_t timestamp, struct NMXVMM2SRSData::VMM2Data * data, size_t elements);

};
