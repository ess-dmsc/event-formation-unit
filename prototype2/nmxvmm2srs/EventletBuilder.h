/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */


#pragma once

#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <NMX/Time.h>
#include <NMX/Geometry.h>
#include <NMX/Clusterer.h>

class EventletBuilder {
public:
  EventletBuilder(Time time_intepreter, Geometry geometry_interpreter);

  /** @todo Martin document */
  uint32_t process_readout(const NMXVMM2SRSData& data, Clusterer& clusterer);

private:
  Time time_intepreter_;
  Geometry geometry_interpreter_;
};
