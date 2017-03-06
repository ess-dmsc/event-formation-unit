/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once

#include <vector>
#include <NMX/Clusterer.h>

class EventletBuilderH5 {
public:
  EventletBuilderH5();

  /** @todo Martin document */
  uint32_t parse(char *buf, size_t size, Clusterer& clusterer);

private:
  size_t psize {sizeof(uint32_t) * 4};
  std::vector<uint32_t> data;

  Eventlet make_eventlet();
};
