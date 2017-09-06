/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once

#include <gdgem/nmx/Clusterer.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/vmm2srs/NMXVMM2SRSData.h>

class EventletBuilder {
public:
  EventletBuilder(SRSTime time_intepreter, SRSMappings geometry_interpreter);

  /** @todo Martin document */
  uint32_t process_readout(NMXVMM2SRSData &data,
                           Clusterer &clusterer,
                           NMXHists &hists);

private:
#ifdef DUMPTOFILE
  int fd;
#endif
  SRSTime time_intepreter_;
  SRSMappings geometry_interpreter_;
};
