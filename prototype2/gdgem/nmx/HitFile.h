/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Hit.h>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

class HitFile
{
public:
  HitFile();

  void open_rw(boost::filesystem::path file_path);
  void open_r(boost::filesystem::path file_path);

  static HitFile create(boost::filesystem::path file_path);
  static HitFile open(boost::filesystem::path file_path);

  size_t count() const;
  void write();
  void read_at(size_t idx, size_t count);

  static void read(std::string file, std::vector<Hit>& external_data);

  std::vector<Hit> data;

  static constexpr size_t chunk_size{9000 / sizeof(Hit)};

private:
  hdf5::file::File file_;
  hdf5::datatype::Datatype dtype_;
  hdf5::node::Dataset dataset_;
  hdf5::dataspace::Hyperslab slab_{{0}, {chunk_size}};
};
