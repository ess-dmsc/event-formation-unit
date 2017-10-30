/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Bastract class for creating NMX eventlets
 */

#include "AbstractEventletBuilder.h"

#include <common/Trace.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cassert>
#include <string>


AbstractBuilder::AbstractBuilder(std::string dump_dir,
                                 bool dump_csv, bool dump_h5)
  : dump_csv_(dump_csv)
  , dump_h5_(dump_h5)
      #ifdef DUMPTOFILE
        , vmmsave{dump_dir + "VMM_", 100000000}
      #endif
{
  if (dump_h5_)
  {
    size_t chunksize = 9000;
    std::string fileName = dump_dir + "VMM3_" + time_str() + ".h5";

    XTRACE(PROCESS, ALW, "Will attempt to dump to H5 file: %s\n",
           fileName.c_str());

    file_ = H5CC::File(fileName, H5CC::Access::rw_truncate);
    if (file_.is_open())
    {
      XTRACE(PROCESS, ALW, "Will dump to H5 file: %s\n",
             fileName.c_str());

      srstime_ = file_.create_dataset<uint32_t>("srs_time",
      {H5CC::kMax}, {chunksize});
      bc_tdc_adc_ = file_.create_dataset<uint16_t>("bc_tdc_adc",
      {H5CC::kMax, 3}, {chunksize, 3});
      fec_chip_chan_thresh_ = file_.create_dataset<uint8_t>("fec_chip_chan_thresh",
      {H5CC::kMax, 4}, {chunksize, 4});
    }
    else
      dump_h5_ = false;
  }
}

std::string AbstractBuilder::time_str()
{
  char cStartTime[50];
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(cStartTime, 50, "%Y-%m-%d-%H-%M-%S", timeinfo);
  std::string startTime = cStartTime;
  return startTime;
}
