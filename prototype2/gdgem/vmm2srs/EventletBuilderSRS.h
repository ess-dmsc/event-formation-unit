/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from SRS/VMM data
 */

#pragma once
#include <common/Trace.h>
#include <gdgem/nmx/AbstractEventletBuilder.h>
#include <gdgem/vmm2srs/ParserSRS.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>

class BuilderSRS : public AbstractBuilder {
public:
  BuilderSRS(SRSTime time_intepreter, SRSMappings geometry_interpreter,
             std::string dump_dir, bool dump_csv, bool dump_h5);

  ~BuilderSRS() { XTRACE(INIT, DEB, "BuilderSRS destructor called"); }

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size, Clusterer &clusterer,
                             NMXHists &hists) override;

private:
  NMXVMM2SRSData parser_;
  SRSTime time_intepreter_;
  SRSMappings geometry_interpreter_;
};
