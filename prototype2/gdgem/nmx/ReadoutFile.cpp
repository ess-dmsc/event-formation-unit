#include <gdgem/nmx/ReadoutFile.h>

#define DATSET_NAME "srs_hits"

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Readout>
{
private:
  static constexpr size_t u8 { sizeof(std::uint8_t)};
  static constexpr size_t u16 { sizeof(std::uint16_t)};
  static constexpr size_t u32 { sizeof(std::uint32_t)};
  static constexpr size_t b { sizeof(bool)};

public:
  using Type = Readout;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Readout));
    type.insert("fec",
                0,
                datatype::create<std::uint8_t>());
    type.insert("chip_id",
                u8,
                datatype::create<std::uint8_t>());
    type.insert("bonus_timestamp",
                2*u8,
                datatype::create<std::uint32_t>());
    type.insert("srs_timestamp",
                2*u8 + u32,
                datatype::create<std::uint32_t>());
    type.insert("channel",
                2*u8 + 2*u32,
                datatype::create<std::uint16_t>());
    type.insert("bcid",
                2*u8 + 2*u32 + u16,
                datatype::create<std::uint16_t>());
    type.insert("tdc",
                2*u8 + 2*u32 + 2*u16,
                datatype::create<std::uint16_t>());
    type.insert("adc",
                2*u8 + 2*u32 + 3*u16,
                datatype::create<std::uint16_t>());
    type.insert("over_threshold",
                2*u8 + 2*u32 + 4*u16,
                datatype::create<bool>());
    return type;
  }
};
}

}

ReadoutFile::ReadoutFile()
{
  dtype_ = hdf5::datatype::create<Readout>();
}

ReadoutFile ReadoutFile::create(boost::filesystem::path file_path)
{
  ReadoutFile ret;
  ret.open_rw(file_path);
  return ret;
}

ReadoutFile ReadoutFile::open(boost::filesystem::path file_path)
{
  ReadoutFile ret;
  ret.open_r(file_path);
  return ret;
}

void ReadoutFile::open_rw(boost::filesystem::path file_path)
{
  using namespace hdf5;

  file_ = file::create(file_path, file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunk_size});

  dataset_ = file_.root().create_dataset(DATSET_NAME, dtype_,
      dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}

void ReadoutFile::open_r(boost::filesystem::path file_path)
{
  using namespace hdf5;

  file_ = file::open(file_path, file::AccessFlags::READONLY);
  dataset_ = file_.root().get_dataset(DATSET_NAME);
}

size_t ReadoutFile::count() const
{
  return hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions().at(0);
}

void ReadoutFile::write()
{
  slab_.offset(0, count());
  slab_.block(0, data.size());
  dataset_.extent({count() + data.size()});
  dataset_.write(data, slab_);
}

void ReadoutFile::read_at(size_t idx, size_t count)
{
  slab_.offset(0, idx);
  slab_.block(0, count);
  data.resize(count);
  dataset_.read(data, slab_);
}

void ReadoutFile::read(std::string file_name, std::vector<Readout>& external_data)
{
  auto file = ReadoutFile::open(file_name);
  file.read_at(0, file.count());
  external_data = std::move(file.data);
}
