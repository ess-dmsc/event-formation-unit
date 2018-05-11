/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/EventletBuilderAPV.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB


BuilderH5::BuilderH5(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  data.resize(4);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
  if (dump_h5_)
    setup_h5(dump_dir);
}

AbstractBuilder::ResultStats BuilderH5::process_buffer(char *buf, size_t size,
                                                       Clusterer &clusterer,
                                                       NMXHists &hists) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    auto eventlet = make_eventlet();
    hists.bin(eventlet);
    clusterer.insert(eventlet);
    buf += psize;
  }
  return AbstractBuilder::ResultStats(count, 0, 0);
}

Eventlet BuilderH5::make_eventlet() {
  Eventlet ret;
  ret.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  ret.plane_id = data[2] >> 16;
  ret.strip = data[2] & 0xFFFF;
  ret.over_threshold = (data[3] >> 16) & 0x1;
  ret.adc = data[3] & 0xFFFF;

  XTRACE(PROCESS, DEB, "Made eventlet: %s\n", ret.debug().c_str());

  if (dump_csv_)
    vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", ret.time, ret.plane_id,
                    ret.strip, ret.adc, ret.over_threshold);

  if (dump_h5_) {
    slab_.offset({event_num_++});
    dataset_.extent({event_num_});
    dataset_.write(ret, slab_);
  }

  return ret;
}

void BuilderH5::setup_h5(std::string dump_dir) {
  size_t chunksize = 9000;
  std::string fileName = dump_dir + "apv2vmm_" + time_str() + ".h5";

  XTRACE(PROCESS, ALW, "Will dump to H5 file: %s\n", fileName.c_str());

  file_ = hdf5::file::create(fileName, hdf5::file::AccessFlags::TRUNCATE);

  hdf5::node::Group root = file_.root();

  hdf5::property::DatasetCreationList dcpl;
  dcpl.layout(hdf5::property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunksize});

  dtype_ = hdf5::datatype::create<Eventlet>();

  dataset_ = root.create_dataset("h5eventlets", dtype_,
      hdf5::dataspace::Simple({0}, {hdf5::dataspace::Simple::UNLIMITED}), dcpl);
}
