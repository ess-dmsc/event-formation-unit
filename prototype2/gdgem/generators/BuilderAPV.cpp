/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderAPV.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderAPV::BuilderAPV(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  data.resize(4);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
  if (dump_h5_) {
    eventlet_file_ = std::make_shared<EventletFile>();
    eventlet_file_->open_rw(dump_dir + "gdgem_apv_" + time_str());
  }
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
    eventlet_file_->data = std::move(converted_data);
    eventlet_file_->write();
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
