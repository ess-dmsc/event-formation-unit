/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading VMM data from HDF5 files
 */

#ifndef READER_VMM_H
#define READER_VMM_H

#include <h5cc/H5CC_DataSet.h>
#include <h5cc/H5CC_File.h>
#include <map>
#include <vector>

class ReaderVMM {
public:
  ReaderVMM(std::string filename);

  size_t read(char *buf);

private:
  H5CC::File file_;
  H5CC::DataSet dataset_;

  size_t total_{0};
  size_t current_{0};

  const std::vector<hsize_t> slabsize{1, H5CC::kMax};
  std::vector<hsize_t> index{0, 0};
  std::vector<uint32_t> data;
};

#endif
