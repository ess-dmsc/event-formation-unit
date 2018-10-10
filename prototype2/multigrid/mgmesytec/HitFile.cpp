/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multigrid/mgmesytec/HitFile.h>

// \todo use string_view when c++17 arrives
constexpr char DatasetName[] = "mgmesytec_hits";

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Multigrid::Hit>
{
public:
  using Type = Multigrid::Hit;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Multigrid::Hit));
    type.insert("trigger_count",
                0,
                datatype::create<size_t>());
    type.insert("external_trigger",
                sizeof(size_t),
                datatype::create<std::int8_t>());
    type.insert("module",
                sizeof(size_t) + sizeof(std::int8_t),
                datatype::create<std::uint8_t>());
    type.insert("high_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t),
                datatype::create<uint32_t>());
    type.insert("low_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                sizeof(std::uint32_t),
                datatype::create<uint32_t>());
    type.insert("total_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                2 * sizeof(std::uint32_t),
                datatype::create<uint64_t>());
    type.insert("bus",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                2 * sizeof(std::uint32_t) + sizeof(std::uint64_t),
                datatype::create<uint8_t>());
    type.insert("channel",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                2 * sizeof(std::uint32_t) + sizeof(std::uint64_t),
                datatype::create<uint16_t>());
    type.insert("adc",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                2 * sizeof(std::uint32_t) + sizeof(std::uint64_t) +
                sizeof(std::uint16_t),
                datatype::create<uint16_t>());
    type.insert("time_diff",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                2 * sizeof(std::uint32_t) + sizeof(std::uint64_t) +
                2 * sizeof(std::uint16_t),
                datatype::create<uint16_t>());
    return type;
  }
};
}

}


namespace Multigrid {

HitFile::HitFile(const boost::filesystem::path& file_path, size_t max_Mb) {
  DataType = hdf5::datatype::create<Hit>();
  MaxSize = max_Mb * 1000000 / sizeof(Hit);
  PathBase = file_path;
}

std::unique_ptr<HitFile> HitFile::create(const boost::filesystem::path& FilePath, size_t MaxMB) {
  auto Ret = std::unique_ptr<HitFile>(new HitFile(FilePath, MaxMB));
  Ret->openRW();
  return Ret;
}

std::unique_ptr<HitFile> HitFile::open(const boost::filesystem::path& FilePath) {
  auto Ret = std::unique_ptr<HitFile>(new HitFile(FilePath, 0));
  Ret->openR();
  return Ret;
}

boost::filesystem::path HitFile::get_full_path() const {
  auto Ret = PathBase;
  if (SequenceNumber > 0)
    Ret += "_" + std::to_string(SequenceNumber);
  Ret += ".h5";
  return Ret;
}

void HitFile::openRW() {
  using namespace hdf5;

  File = file::create(get_full_path(), file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({ChunkSize});

  DataSet = File.root().create_dataset(DatasetName, DataType,
                                         dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}

void HitFile::openR() {
  using namespace hdf5;

  File = file::open(get_full_path(), file::AccessFlags::READONLY);
  DataSet = File.root().get_dataset(DatasetName);
}

size_t HitFile::count() const {
  return hdf5::dataspace::Simple(DataSet.dataspace()).current_dimensions().at(0);
}

void HitFile::write() {
  Slab.offset(0, count());
  Slab.block(0, Data.size());
  DataSet.extent({count() + Data.size()});
  DataSet.write(Data, Slab);

  if (MaxSize && (count() >= MaxSize)) {
    SequenceNumber++;
    openRW();
  }
}

void HitFile::readAt(size_t Index, size_t Count) {
  Slab.offset(0, Index);
  Slab.block(0, Count);
  Data.resize(Count);
  DataSet.read(Data, Slab);
}

void HitFile::read(const boost::filesystem::path &FilePath, std::vector<Hit> &ExternalData) {
  auto TempFile = HitFile::open(FilePath);
  TempFile->readAt(0, TempFile->count());
  ExternalData = std::move(TempFile->Data);
}

void HitFile::push(const std::vector<Hit>& Hits) {
  Data.insert(Data.end(), Hits.begin(), Hits.end());
  if (Data.size() >= ChunkSize) {
    write();
    Data.clear();
  }
}


}