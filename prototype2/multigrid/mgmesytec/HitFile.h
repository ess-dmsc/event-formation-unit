/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multigrid/mgmesytec/Hit.h>
#include <vector>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

// \todo improve reading for multiple files

namespace Multigrid {

class HitFile {
public:

  static std::unique_ptr<HitFile>
  create(const boost::filesystem::path &file_path, size_t max_Mb = 0);

  static std::unique_ptr<HitFile>
  open(const boost::filesystem::path &file_path);

  size_t count() const;
  void read_at(size_t idx, size_t count);

  void push(const std::vector<Hit>& hits);

  static void read(std::string file, std::vector<Hit> &external_data);

  std::vector<Hit> data;

private:
  HitFile(const boost::filesystem::path &file_path, size_t max_Mb);

  static constexpr size_t chunk_size{9000 / sizeof(Hit)};

  hdf5::file::File file_;
  hdf5::datatype::Datatype dtype_;
  hdf5::node::Dataset dataset_;
  hdf5::dataspace::Hyperslab slab_{{0}, {chunk_size}};

  boost::filesystem::path path_base_{};
  size_t max_size_{0};
  size_t sequence_number_{0};

  void open_rw();
  void open_r();
  boost::filesystem::path get_full_path() const;

  void write();

};

}
