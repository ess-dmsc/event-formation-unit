/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Assert.h>
#include <common/Version.h>
#include <fmt/format.h>
#include <memory>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

#define H5_COMPOUND_DEFINE_TYPE(x)                                             \
  using Type = x;                                                              \
  using TypeClass = Compound;                                                  \
  static TypeClass create(const Type & = Type())
#define H5_COMPOUND_INIT auto type = datatype::Compound::create(sizeof(Type))
#define H5_COMPOUND_INSERT_MEMBER(member)                                      \
  type.insert(STRINGIFY(member), offsetof(Type, member),                       \
              datatype::create<decltype(Type::member)>())
#define H5_COMPOUND_RETURN return type

// Returns number of elements in c-style array[]
// from https://www.g-truc.net/post-0708.html
template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept {
  return N;
}

enum class H5PrimSubset {
  Bool,
  Float,
  UInt64,
  UInt32,
  UInt16,
  UInt8,
  SInt8,
  Count
};

template <typename T> struct MapPrimToH5PrimSubset;
template <> struct MapPrimToH5PrimSubset<bool> {
  static constexpr H5PrimSubset Value = H5PrimSubset::Bool;
};
template <> struct MapPrimToH5PrimSubset<float> {
  static constexpr H5PrimSubset Value = H5PrimSubset::Float;
};
template <> struct MapPrimToH5PrimSubset<uint64_t> {
  static constexpr H5PrimSubset Value = H5PrimSubset::UInt64;
};
template <> struct MapPrimToH5PrimSubset<uint32_t> {
  static constexpr H5PrimSubset Value = H5PrimSubset::UInt32;
};
template <> struct MapPrimToH5PrimSubset<uint16_t> {
  static constexpr H5PrimSubset Value = H5PrimSubset::UInt16;
};
template <> struct MapPrimToH5PrimSubset<uint8_t> {
  static constexpr H5PrimSubset Value = H5PrimSubset::UInt8;
};
template <> struct MapPrimToH5PrimSubset<int8_t> {
  static constexpr H5PrimSubset Value = H5PrimSubset::SInt8;
};

struct H5PrimDef {
  const char *Name;
  size_t Offset;
  H5PrimSubset PrimType;
};

struct H5PrimCompoundDef {
  size_t StructSize;
  const H5PrimDef *Members;
  size_t MembersCount;
  const char *DatasetName;
  uint16_t FormatVersion;
};

// traits-like class containing H5PrimCompoundDef.
template <typename T> struct H5PrimCompoundDefData;

// clang-format off
#define H5_PRIM_COMPOUND_BEGIN(type)                                           \
  template <> struct H5PrimCompoundDefData<type> {                             \
    using Type = type;                                                         \
    static const H5PrimCompoundDef &GetCompoundDef() {                         \
      static const H5PrimDef Members[] = {

#define H5_PRIM_COMPOUND_MEMBER(member)                                        \
        { STRINGIFY(member), offsetof(Type, member), MapPrimToH5PrimSubset<decltype(Type::member)>::Value },

#define H5_PRIM_COMPOUND_END() \
      }; \
      static const H5PrimCompoundDef Def{ sizeof(Type), Members, countof(Members), \
        Type::DatasetName(), Type::FormatVersion() }; \
      return Def; \
    } \
  };
// clang-format on

//-----------------------------------------------------------------------------

class PrimDumpFileBase {
public:
  const H5PrimCompoundDef &CompoundDef;

  size_t ChunkSize;

  hdf5::file::File File;
  hdf5::datatype::Compound Compound;
  hdf5::node::Dataset DataSet;
  hdf5::dataspace::Hyperslab Slab;

  boost::filesystem::path PathBase{};
  size_t MaxSize{0};
  size_t SequenceNumber{0};

  boost::filesystem::path get_full_path() const;

  size_t count() const;

  void rotate();

  void write(const void *DataBuffer, size_t DataElmCount);

  void readAt(void *DataBuffer, const size_t DataElmCount, size_t Index,
              size_t Count);

protected:
  PrimDumpFileBase(const H5PrimCompoundDef &PrimStruct,
                   const boost::filesystem::path &file_path, size_t max_Mb);
  void openRW();
  void openR();
};

//-----------------------------------------------------------------------------

template <typename T> struct PrimDumpFile : public PrimDumpFileBase {

  std::vector<T> Data;

  static std::unique_ptr<PrimDumpFile>
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<PrimDumpFile>
  open(const boost::filesystem::path &FilePath);

  void push(const T &Hit);
  template <typename Container> void push(const Container &Hits);

  void readAt(size_t Index, size_t Count);

  static void read(const boost::filesystem::path &FilePath,
                   std::vector<T> &ExternalData);

  void flush();

  void write();

  void rotate();

private:
  PrimDumpFile(const boost::filesystem::path &file_path, size_t max_Mb);
};

//-----------------------------------------------------------------------------

template <typename T>
PrimDumpFile<T>::PrimDumpFile(const boost::filesystem::path &file_path,
                              size_t max_Mb)
    : PrimDumpFileBase(H5PrimCompoundDefData<T>::GetCompoundDef(), file_path,
                       max_Mb) {}

template <typename T>
std::unique_ptr<PrimDumpFile<T>>
PrimDumpFile<T>::create(const boost::filesystem::path &FilePath, size_t MaxMB) {
  auto Ret = std::unique_ptr<PrimDumpFile>(new PrimDumpFile(FilePath, MaxMB));
  Ret->openRW();
  return Ret;
}

template <typename T>
std::unique_ptr<PrimDumpFile<T>>
PrimDumpFile<T>::open(const boost::filesystem::path &FilePath) {
  auto Ret = std::unique_ptr<PrimDumpFile>(new PrimDumpFile(FilePath, 0));
  Ret->openR();
  return Ret;
}

template <typename T> void PrimDumpFile<T>::rotate() {
  SequenceNumber++;
  openRW();
}

template <typename T> void PrimDumpFile<T>::push(const T &Hit) {
  Data.push_back(Hit);
  if (Data.size() >= ChunkSize) {
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}

template <typename T>
template <typename Container>
void PrimDumpFile<T>::push(const Container &Hits) {
  Data.insert(Data.end(), Hits.begin(), Hits.end());
  if (Data.size() >= ChunkSize) {
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}

template <typename T> void PrimDumpFile<T>::readAt(size_t Index, size_t Count) {
  Data.resize(Count);
  PrimDumpFileBase::readAt(Data.data(), Data.size(), Index, Count);
}

template <typename T> void PrimDumpFile<T>::flush() {
  write();
  Data.clear();
}

template <typename T>
void PrimDumpFile<T>::read(const boost::filesystem::path &FilePath,
                           std::vector<T> &ExternalData) {
  auto TempFile = PrimDumpFile::open(FilePath);
  TempFile->readAt(0, TempFile->count());
  ExternalData = std::move(TempFile->Data);
}

template <typename T> void PrimDumpFile<T>::write() {
  PrimDumpFileBase::write(Data.data(), Data.size());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// \todo improve reading for multiple files
template <typename T> class DumpFile {
public:
  ~DumpFile();

  static std::unique_ptr<DumpFile>
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<DumpFile>
  open(const boost::filesystem::path &FilePath);

  size_t count() const;

  void push(const T &Hit);
  template <typename Container> void push(const Container &Hits);

  void readAt(size_t Index, size_t Count);
  static void read(const boost::filesystem::path &FilePath,
                   std::vector<T> &ExternalData);

  /// \todo 9000 is MTU? Correct size is <= 8972 else packet
  /// fragmentation will occur.
  static constexpr size_t ChunkSize{9000 / sizeof(T)};

  boost::filesystem::path get_full_path() const;

  void flush();
  void rotate();

  std::vector<T> Data;

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

  void write();
};

//-----------------------------------------------------------------------------

template <typename T> DumpFile<T>::~DumpFile() {
  if (Data.size() && File.is_valid() &&
      (File.intent() != hdf5::file::AccessFlags::READONLY)) {
    flush();
  }
}

template <typename T>
DumpFile<T>::DumpFile(const boost::filesystem::path &file_path, size_t max_Mb) {
  DataType = hdf5::datatype::create<T>();
  MaxSize = max_Mb * 1000000 / sizeof(T);
  PathBase = file_path;
}

template <typename T>
std::unique_ptr<DumpFile<T>>
DumpFile<T>::create(const boost::filesystem::path &FilePath, size_t MaxMB) {
  auto Ret = std::unique_ptr<DumpFile>(new DumpFile(FilePath, MaxMB));
  Ret->openRW();
  return Ret;
}

template <typename T>
std::unique_ptr<DumpFile<T>>
DumpFile<T>::open(const boost::filesystem::path &FilePath) {
  auto Ret = std::unique_ptr<DumpFile>(new DumpFile(FilePath, 0));
  Ret->openR();
  return Ret;
}

template <typename T>
boost::filesystem::path DumpFile<T>::get_full_path() const {
  auto Ret = PathBase;
  Ret += ("_" + fmt::format("{:0>5}", SequenceNumber) + ".h5");
  return Ret;
}

template <typename T> void DumpFile<T>::openRW() {
  using namespace hdf5;

  File = file::create(get_full_path(), file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({ChunkSize});

  DataSet = File.root().create_dataset(
      T::DatasetName(), DataType,
      dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
  DataSet.attributes.create_from("format_version", T::FormatVersion());
  DataSet.attributes.create_from("EFU_version", efu_version());
  DataSet.attributes.create_from("EFU_build_string", efu_buildstr());
}

template <typename T> void DumpFile<T>::rotate() {
  SequenceNumber++;
  openRW();
}

template <typename T> void DumpFile<T>::openR() {
  using namespace hdf5;

  auto Path = PathBase;
  Path += ".h5";

  File = file::open(Path, file::AccessFlags::READONLY);
  DataSet = File.root().get_dataset(T::DatasetName());
  // if not initial version, expect it to be well formed
  if (T::FormatVersion() > 0) {
    uint16_t ver{0};
    DataSet.attributes["format_version"].read(ver);
    if (ver != T::FormatVersion()) {
      throw std::runtime_error("DumpFile version mismatch");
    }
  }
}

template <typename T> size_t DumpFile<T>::count() const {
  return hdf5::dataspace::Simple(DataSet.dataspace())
      .current_dimensions()
      .at(0);
}

template <typename T> void DumpFile<T>::write() {
  Slab.offset(0, count());
  Slab.block(0, Data.size());
  DataSet.extent({count() + Data.size()});
  DataSet.write(Data, Slab);
}

template <typename T> void DumpFile<T>::flush() {
  write();
  Data.clear();
}

template <typename T> void DumpFile<T>::readAt(size_t Index, size_t Count) {
  Slab.offset(0, Index);
  Slab.block(0, Count);
  Data.resize(Count);
  DataSet.read(Data, Slab);
}

template <typename T>
void DumpFile<T>::read(const boost::filesystem::path &FilePath,
                       std::vector<T> &ExternalData) {
  auto TempFile = DumpFile::open(FilePath);
  TempFile->readAt(0, TempFile->count());
  ExternalData = std::move(TempFile->Data);
}

template <typename T> void DumpFile<T>::push(const T &Hit) {
  Data.push_back(Hit);
  if (Data.size() >= ChunkSize) {
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}

template <typename T>
template <typename Container>
void DumpFile<T>::push(const Container &Hits) {
  Data.insert(Data.end(), Hits.begin(), Hits.end());
  if (Data.size() >= ChunkSize) {
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}
