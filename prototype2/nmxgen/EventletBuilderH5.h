/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from h5 data
 */

#pragma once

#include <NMX/Clusterer.h>
#include <vector>

class EventletBuilderH5 {
public:
  EventletBuilderH5();

  /** @todo Martin document */
  uint32_t process_readout(char *buf, size_t size, Clusterer &clusterer);

private:
  size_t psize{sizeof(uint32_t) * 4};
  std::vector<uint32_t> data;

  Eventlet make_eventlet();
};
