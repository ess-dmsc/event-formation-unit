/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>
#include <memory>
#include <common/Version.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

#define H5_COMPOUND_DEFINE_TYPE(x) using Type = x; using TypeClass = Compound; static TypeClass create(const Type & = Type())
#define H5_COMPOUND_INIT auto type = datatype::Compound::create(sizeof(Type))
#define H5_COMPOUND_INSERT_MEMBER(member) type.insert(STRINGIFY(member), offsetof(Type, member), datatype::create<decltype(Type::member)>())
#define H5_COMPOUND_RETURN return type

// \todo improve reading for multiple files
template<typename T>
class DumpFile {
public:
  static std::unique_ptr<DumpFile>
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<DumpFile>
  open(const boost::filesystem::path &FilePath);

  size_t count() const;

  void push(const T& Hit);
  void push(const std::vector<T>& Hits);

  void readAt(size_t Index, size_t Count);
  static void read(const boost::filesystem::path &FilePath,
      std::vector<T> &ExternalData);

  std::vector<T> Data;

  static constexpr size_t ChunkSize{9000 / sizeof(T)};

private:
  DumpFile(const boost::filesystem::path &file_path, size_t max_Mb);

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

template<typename T>
DumpFile<T>::DumpFile(const boost::filesystem::path& file_path, size_t max_Mb) {
  DataType = hdf5::datatype::create<T>();
  MaxSize = max_Mb * 1000000 / sizeof(T);
  PathBase = file_path;
}

template<typename T>
std::unique_ptr<DumpFile<T>> DumpFile<T>::create(const boost::filesystem::path& FilePath, size_t MaxMB) {
  auto Ret = std::unique_ptr<DumpFile>(new DumpFile(FilePath, MaxMB));
  Ret->openRW();
  return Ret;
}

template<typename T>
std::unique_ptr<DumpFile<T>> DumpFile<T>::open(const boost::filesystem::path& FilePath) {
  auto Ret = std::unique_ptr<DumpFile>(new DumpFile(FilePath, 0));
  Ret->openR();
  return Ret;
}

template<typename T>
boost::filesystem::path DumpFile<T>::get_full_path() const {
  auto Ret = PathBase;
  if (SequenceNumber > 0)
    Ret += "_" + std::to_string(SequenceNumber);
  Ret += ".h5";
  return Ret;
}

template<typename T>
void DumpFile<T>::openRW() {
  using namespace hdf5;

  File = file::create(get_full_path(), file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({ChunkSize});

  DataSet = File.root().create_dataset(T::DatasetName(), DataType,
                                       dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
  DataSet.attributes.create_from("format_version", T::FormatVersion());
  DataSet.attributes.create_from("EFU_version", efu_version());
  DataSet.attributes.create_from("EFU_build_string", efu_buildstr());
}

template<typename T>
void DumpFile<T>::openR() {
  using namespace hdf5;

  File = file::open(get_full_path(), file::AccessFlags::READONLY);
  DataSet = File.root().get_dataset(T::DatasetName());
  // if not initial version, expect it to be well formed
  if (T::FormatVersion() > 0) {
    uint16_t ver {0};
    DataSet.attributes["format_version"].read(ver);
    if (ver != T::FormatVersion()) {
      throw std::runtime_error("DumpFile version mismatch");
    }
  }
}

template<typename T>
size_t DumpFile<T>::count() const {
  return hdf5::dataspace::Simple(DataSet.dataspace()).current_dimensions().at(0);
}

template<typename T>
void DumpFile<T>::write() {
  Slab.offset(0, count());
  Slab.block(0, Data.size());
  DataSet.extent({count() + Data.size()});
  DataSet.write(Data, Slab);

  if (MaxSize && (count() >= MaxSize)) {
    SequenceNumber++;
    openRW();
  }
}

template<typename T>
void DumpFile<T>::readAt(size_t Index, size_t Count) {
  Slab.offset(0, Index);
  Slab.block(0, Count);
  Data.resize(Count);
  DataSet.read(Data, Slab);
}

template<typename T>
void DumpFile<T>::read(const boost::filesystem::path &FilePath, std::vector<T> &ExternalData) {
  auto TempFile = DumpFile::open(FilePath);
  TempFile->readAt(0, TempFile->count());
  ExternalData = std::move(TempFile->Data);
}

template<typename T>
void DumpFile<T>::push(const T& Hit) {
  Data.push_back(Hit);
  if (Data.size() >= ChunkSize) {
    write();
    Data.clear();
  }
}

template<typename T>
void DumpFile<T>::push(const std::vector<T>& Hits) {
  Data.insert(Data.end(), Hits.begin(), Hits.end());
  if (Data.size() >= ChunkSize) {
    write();
    Data.clear();
  }
}
