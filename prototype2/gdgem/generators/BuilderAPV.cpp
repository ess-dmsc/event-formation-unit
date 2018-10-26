/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/BuilderAPV.h>
#include <common/TimeString.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderAPV::BuilderAPV(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  data.resize(4);
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc\n");
  if (dump_h5_) {
    hit_file_ = std::make_shared<HitFile>();
    hit_file_->open_rw(dump_dir + "gdgem_apv2vmm_" + timeString());
  }
}

AbstractBuilder::ResultStats BuilderAPV::process_buffer(char *buf, size_t size) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  converted_data.resize(count);
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    make_hit(i);
    buf += psize;
  }

  std::vector<Hit> converted_x;
  std::vector<Hit> converted_y;

  for (const auto& e : converted_data) {
    if (e.plane_id)
      converted_y.push_back(e);
    else
      converted_x.push_back(e);
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

void BuilderAPV::make_hit(size_t idx) {
  auto& e = converted_data[idx];
  e.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  e.plane_id = data[2] >> 16;
  e.strip = data[2] & 0xFFFF;
  e.adc = data[3] & 0xFFFF;

  XTRACE(PROCESS, DEB, "Made hit: %s", e.debug().c_str());

  if (dump_csv_)
    vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", e.time, e.plane_id,
                    e.strip, e.adc);
}
