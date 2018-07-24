/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading VMM data from HDF5 files
///
//===----------------------------------------------------------------------===//
/// GCOVR_EXCL_START

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

#include <map>
#include <vector>

/// Assuming MTU has been set to 9000 bytes
/// Max UDP is then: MTU  - IP_HEADER - UDP_HEADER
///                  9000 - 20        - 8
const int maxUdpPayloadSize{8972};

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
  size_t max_in_buf_{maxUdpPayloadSize / (sizeof(uint32_t) * 4)};

  hdf5::dataspace::Hyperslab slab_{{0, 0}, {1, 4}};

  std::vector<uint32_t> data;
};
/// GCOVR_EXCL_STOP
