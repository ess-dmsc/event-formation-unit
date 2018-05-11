/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading VMM data from HDF5 files
 */

#ifndef READER_VMM_H
#define READER_VMM_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

#include <map>
#include <vector>

class ReaderAPV {
public:
  /** @todo document */
  ReaderAPV(std::string filename);

  /** @todo document */
  size_t read(char *buf);

private:
  hdf5::file::File file_;
  hdf5::node::Dataset dataset_;

  size_t total_{0};
  size_t current_{0};
  size_t psize_{sizeof(uint32_t) * 4};
  size_t max_in_buf_{9000 / (sizeof(uint32_t) * 4)};

  hdf5::dataspace::Hyperslab slab_{{0, 0}, {1, 4}};

  std::vector<uint32_t> data;
};

#endif
