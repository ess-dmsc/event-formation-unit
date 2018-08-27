/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multigrid/mgmesytec/HitFile.h>

#define DATSET_NAME "mgmesytec_hits"

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
  dtype_ = hdf5::datatype::create<Hit>();
  max_size_ = max_Mb * 1000000 / sizeof(Hit);
  path_base_ = file_path;
}

std::unique_ptr<HitFile> HitFile::create(const boost::filesystem::path& file_path, size_t max_Mb) {
  auto ret = std::unique_ptr<HitFile>(new HitFile(file_path, max_Mb));
  ret->open_rw();
  return ret;
}

std::unique_ptr<HitFile> HitFile::open(const boost::filesystem::path& file_path) {
  auto ret = std::unique_ptr<HitFile>(new HitFile(file_path, 0));
  ret->open_r();
  return ret;
}

boost::filesystem::path HitFile::get_full_path() const {
  auto ret = path_base_;
  if (sequence_number_ > 0)
    ret += "_" + std::to_string(sequence_number_);
  ret += ".h5";
  return ret;
}

void HitFile::open_rw() {
  using namespace hdf5;

  file_ = file::create(get_full_path(), file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunk_size});

  dataset_ = file_.root().create_dataset(DATSET_NAME, dtype_,
                                         dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}

void HitFile::open_r() {
  using namespace hdf5;

  file_ = file::open(get_full_path(), file::AccessFlags::READONLY);
  dataset_ = file_.root().get_dataset(DATSET_NAME);
}

size_t HitFile::count() const {
  return hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions().at(0);
}

void HitFile::write() {
  slab_.offset(0, count());
  slab_.block(0, data.size());
  dataset_.extent({count() + data.size()});
  dataset_.write(data, slab_);

  if (max_size_ && (count() >= max_size_)) {
    sequence_number_++;
    open_rw();
  }
}

void HitFile::read_at(size_t idx, size_t count) {
  slab_.offset(0, idx);
  slab_.block(0, count);
  data.resize(count);
  dataset_.read(data, slab_);
}

void HitFile::read(std::string file_name, std::vector<Hit> &external_data) {
  auto file = HitFile::open(file_name);
  file->read_at(0, file->count());
  external_data = std::move(file->data);
}

void HitFile::push(const std::vector<Hit>& hits) {
  data.insert(data.end(), hits.begin(), hits.end());
  if (data.size() >= chunk_size) {
    write();
    data.clear();
  }
}


}