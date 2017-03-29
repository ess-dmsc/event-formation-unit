/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once

#include <NMX/Clusterer.h>
#include <NMX/Geometry.h>
#include <NMX/Time.h>
#include <nmxvmm2srs/NMXVMM2SRSData.h>

class EventletBuilder {
public:
  EventletBuilder(Time time_intepreter, Geometry geometry_interpreter);

  /** @todo Martin document */
  uint32_t process_readout(NMXVMM2SRSData &data, Clusterer &clusterer);

private:
  int fd;
  Time time_intepreter_;
  Geometry geometry_interpreter_;
};
