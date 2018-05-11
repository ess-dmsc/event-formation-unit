/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/EventletBuilderAPV.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderAPV::BuilderAPV(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  data.resize(4);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
  if (dump_h5_)
    setup_h5(dump_dir);
}

AbstractBuilder::ResultStats BuilderAPV::process_buffer(char *buf, size_t size,
                                                       Clusterer &clusterer,
                                                       NMXHists &hists) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    make_eventlet(i);
    buf += psize;
  }

  for (const auto& e : converted_data) {
    hists.bin(e);
    clusterer.insert(e);
  }

  if (dump_h5_) {
    write_h5();
  }

  return AbstractBuilder::ResultStats(count, 0, 0);
}

void BuilderAPV::make_eventlet(size_t idx) {
  auto& e = converted_data[idx];
  e.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  e.plane_id = data[2] >> 16;
  e.strip = data[2] & 0xFFFF;
  e.over_threshold = (data[3] >> 16) & 0x1;
  e.adc = data[3] & 0xFFFF;

  XTRACE(PROCESS, DEB, "Made eventlet: %s\n", e.debug().c_str());

  if (dump_csv_)
    vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", e.time, e.plane_id,
                    e.strip, e.adc, e.over_threshold);
}

void BuilderAPV::write_h5() {
  slab_.offset(0, event_num_);
  slab_.block(0, converted_data.size());
  dataset_.extent({event_num_ + converted_data.size()});
  dataset_.write(converted_data, slab_);
  event_num_ += converted_data.size();
}

void BuilderAPV::setup_h5(std::string dump_dir) {
  using namespace hdf5;

  converted_data.resize(9000 / sizeof(Eventlet));

  std::string fileName = dump_dir + "apv2vmm_" + time_str() + ".h5";

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
