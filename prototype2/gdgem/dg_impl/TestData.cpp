#include <gdgem/dg_impl/TestData.h>

void SRSHitIO::read(std::string file_name)
{
  using namespace hdf5;

  auto f = file::open(file_name, file::AccessFlags::READONLY);
  node::Group root = f.root();

  auto dataset = root.get_dataset("srs_hits");
  auto shape = hdf5::dataspace::Simple(dataset.dataspace()).current_dimensions();

  data.resize(shape[0]);
  dataset.read(data);
}

void SRSHitIO::write(std::string file_name)
{
  using namespace hdf5;

  auto f = file::create(file_name, file::AccessFlags::TRUNCATE);
  node::Group root = f.root();

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({9000 / sizeof(Hit)});

  auto dtype = datatype::create<Hit>();

  auto dataset = root.create_dataset("srs_hits", dtype,
      dataspace::Simple({data.size()}), dcpl);

  dataset.write(data);
}
