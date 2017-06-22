/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once

#include <gdgem/nmx/Clusterer.h>
#include <gdgem/nmx/Geometry.h>
#include <gdgem/nmx/Time.h>
#include <gdgem/vmm2srs/NMXVMM2SRSData.h>

class EventletBuilder {
public:
  EventletBuilder(Time time_intepreter, Geometry geometry_interpreter);

  /** @todo Martin document */
  uint32_t process_readout(NMXVMM2SRSData &data, Clusterer &clusterer);

private:
#ifdef DUMPTOFILE
  int fd;
#endif
  Time time_intepreter_;
  Geometry geometry_interpreter_;
};
