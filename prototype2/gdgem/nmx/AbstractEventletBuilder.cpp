/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Bastract class for creating NMX eventlets
 */

#include "AbstractEventletBuilder.h"

#include <common/Trace.h>

AbstractBuilder::AbstractBuilder(std::string __attribute__((unused)) dump_dir,
                                 bool dump_csv, bool dump_h5)
  : dump_csv_(dump_csv)
  , dump_h5_(dump_h5)
{
#ifdef DUMPTOFILE
  if (dump_h5_) {
    setup_h5(dump_dir);
  }
  if (dump_csv_) {
    vmmsave = std::make_shared<DataSave>(dump_dir + "VMM_", 100000000);
  }
#endif
}


#ifdef DUMPTOFILE
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

void AbstractBuilder::setup_h5(std::string dump_dir)
{
  size_t chunksize = 9000;
  std::string fileName = dump_dir + "VMM3_" + time_str() + ".h5";

  XTRACE(PROCESS, ALW, "Will dump to H5 file: %s\n",
         fileName.c_str());

  file_ = hdf5::file::open(fileName, hdf5::file::AccessFlags::TRUNCATE);

  hdf5::node::Group root = file_.root();

  hdf5::property::LinkCreationList lcpl;
  hdf5::property::DatasetCreationList dcpl;
  dcpl.layout(hdf5::property::DatasetLayout::CHUNKED);

  dcpl.chunk({chunksize});
  srstime_
      = root.create_dataset("srs_time",
                            hdf5::datatype::create<uint32_t>(),
                            hdf5::dataspace::Simple({chunksize}, {hdf5::dataspace::Simple::UNLIMITED}),
                            lcpl, dcpl);


  dcpl.chunk({chunksize, 3});
  bc_tdc_adc_
      = root.create_dataset("bc_tdc_adc",
                            hdf5::datatype::create<uint16_t>(),
                            hdf5::dataspace::Simple({chunksize, 3}, {hdf5::dataspace::Simple::UNLIMITED, 3}),
                            lcpl, dcpl);

  dcpl.chunk({chunksize, 4});
  fec_chip_chan_thresh_
      = root.create_dataset("fec_chip_chan_thresh",
                            hdf5::datatype::create<uint8_t>(),
                            hdf5::dataspace::Simple({chunksize, 4}, {hdf5::dataspace::Simple::UNLIMITED, 4}),
                            lcpl, dcpl);
}
#endif
