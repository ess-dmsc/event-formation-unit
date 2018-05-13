#include <gdgem/nmx/EventletFile.h>

#define DATSET_NAME "gdgem_eventlets"

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Eventlet>
{
public:
  using Type = Eventlet;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Eventlet));
    type.insert("time",
                0,
                datatype::create<double>());
    type.insert("plane_id",
                sizeof(double),
                datatype::create<std::uint8_t>());
    type.insert("strip",
                sizeof(double) + sizeof(std::uint8_t),
                datatype::create<Eventlet::strip_type>());
    type.insert("adc",
                sizeof(double) + sizeof(std::uint8_t) + sizeof(Eventlet::strip_type),
                datatype::create<Eventlet::adc_type>());
    type.insert("over_threshold",
                sizeof(double) + sizeof(std::uint8_t) + sizeof(Eventlet::strip_type)
                    + sizeof(Eventlet::adc_type),
                datatype::create<bool>());
    return type;
  }
};
}

}


EventletFile::EventletFile()
{
  dtype_ = hdf5::datatype::create<Eventlet>();
}

EventletFile EventletFile::create(boost::filesystem::path file_path)
{
  EventletFile ret;
  ret.open_rw(file_path);
  return ret;
}

EventletFile EventletFile::open(boost::filesystem::path file_path)
{
  EventletFile ret;
  ret.open_r(file_path);
  return ret;
}

void EventletFile::open_rw(boost::filesystem::path file_path)
{
  using namespace hdf5;

  file_ = file::create(file_path, file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunk_size});

  dataset_ = file_.root().create_dataset(DATSET_NAME, dtype_,
      dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}

void EventletFile::open_r(boost::filesystem::path file_path)
{
  using namespace hdf5;

  file_ = file::open(file_path, file::AccessFlags::READONLY);
  dataset_ = file_.root().get_dataset(DATSET_NAME);
}

size_t EventletFile::count() const
{
  return hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions().at(0);
}

void EventletFile::write()
{
  slab_.offset(0, count());
  slab_.block(0, data.size());
  dataset_.extent({count() + data.size()});
  dataset_.write(data, slab_);
}

void EventletFile::read_at(size_t idx, size_t count)
{
  slab_.offset(0, idx);
  slab_.block(0, count);
  data.resize(count);
  dataset_.read(data, slab_);
}

void EventletFile::read(std::string file_name, std::vector<Eventlet>& external_data)
{
  auto file = EventletFile::open(file_name);
  file.read_at(0, file.count());
  external_data = std::move(file.data);
}
