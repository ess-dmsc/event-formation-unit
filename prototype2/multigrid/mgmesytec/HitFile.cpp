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

HitFile::HitFile() {
  dtype_ = hdf5::datatype::create<Hit>();
}

HitFile HitFile::create(boost::filesystem::path file_path) {
  HitFile ret;
  ret.open_rw(file_path);
  return ret;
}

HitFile HitFile::open(boost::filesystem::path file_path) {
  HitFile ret;
  ret.open_r(file_path);
  return ret;
}

void HitFile::open_rw(boost::filesystem::path file_path) {
  using namespace hdf5;

  file_ = file::create(file_path, file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunk_size});

  dataset_ = file_.root().create_dataset(DATSET_NAME, dtype_,
                                         dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}

void HitFile::open_r(boost::filesystem::path file_path) {
  using namespace hdf5;

  file_ = file::open(file_path, file::AccessFlags::READONLY);
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
}

void HitFile::read_at(size_t idx, size_t count) {
  slab_.offset(0, idx);
  slab_.block(0, count);
  data.resize(count);
  dataset_.read(data, slab_);
}

void HitFile::read(std::string file_name, std::vector<Hit> &external_data) {
  auto file = HitFile::open(file_name);
  file.read_at(0, file.count());
  external_data = std::move(file.data);
}

}