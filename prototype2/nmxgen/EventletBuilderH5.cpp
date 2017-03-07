/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <nmxgen/EventletBuilderH5.h>

EventletBuilderH5::EventletBuilderH5() { data.resize(4); }

uint32_t EventletBuilderH5::process_readout(char *buf, size_t size,
                                            Clusterer &clusterer) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    clusterer.insert(make_eventlet());
    buf += psize;
  }
  return count;
}

Eventlet EventletBuilderH5::make_eventlet() {
  Eventlet ret;
  ret.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  ret.plane_id = data[2] >> 16;
  ret.strip = data[2] & 0xFFFF;
  ret.flag = (data[3] >> 16) & 0x1;
  ret.over_threshold = (data[3] >> 17) & 0x1;
  ret.adc = data[3] & 0xFFFF;
  return ret;
}
