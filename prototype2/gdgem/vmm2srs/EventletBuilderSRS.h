/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once
#include <common/Trace.h>
#include <gdgem/nmx/AbstractEventletBuilder.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/vmm2srs/ParserSRS.h>
#include <dataformats/multigrid/inc/DataSave.h>

class BuilderSRS : public AbstractBuilder {
public:
  BuilderSRS(SRSTime time_intepreter, SRSMappings geometry_interpreter);

  ~BuilderSRS(){
    XTRACE(INIT, DEB, "BuilderSRS destructor called\n");
  }

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size,
                             Clusterer &clusterer,
                             NMXHists &hists) override;

private:
  #ifdef DUMPTOFILE
    DataSave vmmsave{"VMM_", 100000000};
  #endif
  NMXVMM2SRSData parser_;
  SRSTime time_intepreter_;
  SRSMappings geometry_interpreter_;
};
