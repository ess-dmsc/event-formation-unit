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
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<HitFile>
  open(const boost::filesystem::path &FilePath);

  size_t count() const;
  void readAt(size_t Index, size_t Count);

  void push(const std::vector<Hit>& Hits);

  static void read(const boost::filesystem::path &FilePath,
      std::vector<Hit> &ExternalData);

  std::vector<Hit> Data;

private:
  HitFile(const boost::filesystem::path &file_path, size_t max_Mb);

  static constexpr size_t ChunkSize{9000 / sizeof(Hit)};

  hdf5::file::File File;
  hdf5::datatype::Datatype DataType;
  hdf5::node::Dataset DataSet;
  hdf5::dataspace::Hyperslab Slab{{0}, {ChunkSize}};

  boost::filesystem::path PathBase{};
  size_t MaxSize{0};
  size_t SequenceNumber{0};

  void openRW();
  void openR();
  boost::filesystem::path get_full_path() const;

  void write();
};

}
