/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <gdgem/nmxgen/EventletBuilderH5.h>
#include <iostream>

BuilderH5::BuilderH5()
  : AbstractBuilder()
{ data.resize(4); }

AbstractBuilder::ResultStats
BuilderH5::process_buffer(char *buf, size_t size,
                          Clusterer &clusterer,
                          NMXHists &hists) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    auto eventlet = make_eventlet();
    clusterer.insert(eventlet);
    hists.bin_one(eventlet.plane_id, eventlet.strip);
    buf += psize;
  }
  return AbstractBuilder::ResultStats(count, 0);
}

Eventlet BuilderH5::make_eventlet() {
  Eventlet ret;
  ret.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  ret.plane_id = data[2] >> 16;
  ret.strip = data[2] & 0xFFFF;
  ret.flag = (data[3] >> 16) & 0x1;
  ret.over_threshold = (data[3] >> 17) & 0x1;
  ret.adc = data[3] & 0xFFFF;

//  std::cout << "Made eventlet: " << ret.debug() << "\n";
  return ret;
}
