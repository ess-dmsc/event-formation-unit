/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading VMM data from HDF5 files
 */

#ifndef READER_VMM_H
#define READER_VMM_H

#include <h5cpp/hdf5.hpp>
//#include <H5CC_DataSet.h>
//#include <H5CC_File.h>
#include <map>
#include <vector>

class ReaderVMM {
public:
  /** @todo document */
  ReaderVMM(std::string filename);

  /** @todo document */
  size_t read(char *buf);

private:
  hdf5::file::File file_;
  hdf5::node::Dataset dataset_;

  size_t total_{0};
  size_t current_{0};
  size_t psize_{sizeof(uint32_t) * 4};
  size_t max_in_buf_{9000 / (sizeof(uint32_t) * 4)};

  hdf5::dataspace::Hyperslab slab_{{0,0},
                                   {1, 4}};

//  std::vector<hsize_t> index{0, 0};
//  std::vector<hsize_t> slabsize{1, H5CC::kMax};

  std::vector<uint32_t> data;
};

#endif
