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
#include <fmt/format.h>

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

hid_t MapPrimToH5tNative(H5PrimSubset Prim) {
  static hid_t PrimToH5tNative[static_cast<int>(H5PrimSubset::Count)] = {};
  static bool Init = true;
  if (Init) {
    Init = false;

    PrimToH5tNative[static_cast<int>(H5PrimSubset::Bool)] = H5T_NATIVE_HBOOL;

    PrimToH5tNative[static_cast<int>(H5PrimSubset::Float)] = H5T_NATIVE_FLOAT;

    PrimToH5tNative[static_cast<int>(H5PrimSubset::UInt64)] = H5T_NATIVE_ULLONG;
    static_assert(sizeof(uint64_t) == sizeof(unsigned long long),
                  "H5 types: Potential 32/64 bit issue.");

    PrimToH5tNative[static_cast<int>(H5PrimSubset::UInt32)] = H5T_NATIVE_UINT;
    static_assert(sizeof(uint32_t) == sizeof(unsigned int),
                  "H5 types: Potential 32/64 bit issue.");

    PrimToH5tNative[static_cast<int>(H5PrimSubset::UInt16)] = H5T_NATIVE_USHORT;
    static_assert(sizeof(uint16_t) == sizeof(unsigned short),
                  "Potential 32/64 bit definition issue.");

    PrimToH5tNative[static_cast<int>(H5PrimSubset::UInt8)] = H5T_NATIVE_UCHAR;
    static_assert(sizeof(uint8_t) == sizeof(unsigned char),
                  "H5 types: Potential 32/64 bit issue.");

    PrimToH5tNative[static_cast<int>(H5PrimSubset::SInt8)] = H5T_NATIVE_SCHAR;
    static_assert(sizeof(int8_t) == sizeof(signed char),
                  "H5 types: Potential 32/64 bit issue.");

    // verify all slots have been written
    for (int i = 0; i < static_cast<int>(H5PrimSubset::Count); i++) {
      if (PrimToH5tNative[i] == 0) {
        throw std::runtime_error(
            fmt::format("Missing H5PrimSubset to H5T_NATIVE_* mapping at "
                        "H5PrimSubset enum item {}",
                        i));
      }
    }
  }
  return PrimToH5tNative[static_cast<int>(Prim)];
}

struct H5PrimDef {
  const char *Name;
  size_t Offset;
  H5PrimSubset PrimType;
};

struct H5PrimCompoundDef{
  size_t StructSize;
  const H5PrimDef* Members;
  size_t MembersCount;
};

// traits-like class containing H5PrimCompoundDef.
template<typename T> struct H5PrimCompoundDefData;


class PrimDumpFileBase{
public:

  boost::filesystem::path PathBase{};
  size_t MaxSize{0};
  
  PrimDumpFileBase(const H5PrimCompoundDef& PrimStruct, const boost::filesystem::path &file_path, size_t max_Mb) {

    hdf5::datatype::Compound type =
        hdf5::datatype::Compound::create(PrimStruct.StructSize);

    for (size_t i = 0; i < PrimStruct.MembersCount; i++) {
      const H5PrimDef &Def = PrimStruct.Members[i];
      hid_t H5fNativeType = MapPrimToH5tNative(Def.PrimType);
      type.insert(
          Def.Name, Def.Offset,
          hdf5::datatype::Datatype(hdf5::ObjectHandle(H5Tcopy(H5fNativeType))));
    }

    MaxSize = max_Mb * 1000000 / PrimStruct.StructSize;
    PathBase = file_path;
  }
};


template<typename T>
struct PrimpDumpFile : public PrimDumpFileBase {
  PrimpDumpFile(const boost::filesystem::path &file_path, size_t max_Mb)
      : PrimDumpFileBase(H5PrimCompoundDefData<T>::GetCompoundDef(), file_path, max_Mb) {}
};








// \todo improve reading for multiple files
template<typename T>
class DumpFile {
public:
  ~DumpFile();

  static std::unique_ptr<DumpFile>
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<DumpFile>
  open(const boost::filesystem::path &FilePath);

  size_t count() const;

  void push(const T& Hit);
  template<typename Container> void push(const Container& Hits);

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

template<typename T>
DumpFile<T>::~DumpFile() {
  if (Data.size() && File.is_valid() &&
      (File.intent() != hdf5::file::AccessFlags::READONLY)) {
    flush();
  }
}

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
  Ret += ("_" + fmt::format("{:0>5}", SequenceNumber) + ".h5");
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
void DumpFile<T>::rotate() {
  SequenceNumber++;
  openRW();
}

template<typename T>
void DumpFile<T>::openR() {
  using namespace hdf5;

  auto Path = PathBase;
  Path += ".h5";

  File = file::open(Path, file::AccessFlags::READONLY);
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
}

template<typename T>
void DumpFile<T>::flush() {
  write();
  Data.clear();
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
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}

template<typename T>
template<typename Container>
void DumpFile<T>::push(const Container& Hits) {
  Data.insert(Data.end(), Hits.begin(), Hits.end());
  if (Data.size() >= ChunkSize) {
    flush();

    if (MaxSize && (count() >= MaxSize)) {
      rotate();
    }
  }
}

