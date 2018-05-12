/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderEventlets.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderEventlets::BuilderEventlets(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  converted_data.reserve(9000 / psize);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
  if (dump_h5_) {
    eventlet_file_ = std::make_shared<EventletFile>();
    eventlet_file_->open_rw(dump_dir + "gdgem_eventlets_" + time_str() + ".h5");
  }
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
    eventlet_file_->data = std::move(converted_data);
    eventlet_file_->write();
  }

  return AbstractBuilder::ResultStats(count, 0, 0);
}
