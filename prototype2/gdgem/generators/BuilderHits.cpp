/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderHits.h>
#include <common/TimeString.h>
#include <cstring>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderHits::BuilderHits(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  converted_data.reserve(9000 / psize);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc\n");
  if (dump_h5_) {
    hit_file_ = std::make_shared<HitFile>();
    hit_file_->open_rw(dump_dir + "gdgem_hits_" + timeString() + ".h5");
  }
}

AbstractBuilder::ResultStats BuilderHits::process_buffer(char *buf, size_t size) {
  size_t count = std::min(size / psize, size_t(9000 / psize));

  converted_data.resize(count);
  memcpy(converted_data.data(), buf, count*psize);

  std::vector<Hit> converted_x;
  std::vector<Hit> converted_y;

  for (const auto& e : converted_data) {
    if (e.plane_id)
      converted_y.push_back(e);
    else
      converted_x.push_back(e);
    if (dump_csv_)
      vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", e.time, e.plane_id,
                      e.strip, e.adc);
  }

  if (dump_h5_) {
    hit_file_->data = std::move(converted_data);
    hit_file_->write();
  }

  if (clusterer_x)
    clusterer_x->cluster(converted_x);
  if (clusterer_y)
    clusterer_y->cluster(converted_y);

  return AbstractBuilder::ResultStats(count, 0, 0);
}
