/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/EventletBuilderEventlets.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderEventlets::BuilderEventlets(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  converted_data.reserve(9000 / psize);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
  if (dump_h5_)
    setup_h5(dump_dir);
}

AbstractBuilder::ResultStats BuilderEventlets::process_buffer(char *buf, size_t size,
                                                       Clusterer &clusterer,
                                                       NMXHists &hists) {
  size_t count = std::min(size / psize, size_t(9000 / psize));

  converted_data.resize(count);
  memcpy(converted_data.data(), buf, count*psize);

  for (const auto& e : converted_data) {
    hists.bin(e);
    clusterer.insert(e);
    if (dump_csv_)
      vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", e.time, e.plane_id,
                      e.strip, e.adc, e.over_threshold);
  }

  if (dump_h5_) {
    write_h5();
  }

  return AbstractBuilder::ResultStats(count, 0, 0);
}

void BuilderEventlets::write_h5() {
  slab_.offset(0, event_num_);
  slab_.block(0, converted_data.size());
  dataset_.extent({event_num_ + converted_data.size()});
  dataset_.write(converted_data, slab_);
  event_num_ += converted_data.size();
}

void BuilderEventlets::setup_h5(std::string dump_dir) {
  using namespace hdf5;

  converted_data.resize(9000 / sizeof(Eventlet));

  std::string fileName = dump_dir + "eventlets_" + time_str() + ".h5";

  XTRACE(PROCESS, ALW, "Will dump to H5 file: %s\n", fileName.c_str());

  file_ = file::create(fileName, file::AccessFlags::TRUNCATE);

  node::Group root = file_.root();

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({converted_data.size()});

  dtype_ = datatype::create<Eventlet>();

  dataset_ = root.create_dataset("h5eventlets", dtype_,
      dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
}
