/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once

#include <NMX/EventNMX.h>
#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <map>

class ParserClusterer {
public:
  ParserClusterer();

  /** @todo Martin document */
  void parse(char *buf, size_t size);

  void parse(unsigned int planeid, uint32_t timestamp, struct NMXVMM2SRSData::VMM2Data * data, size_t elements);

  /** @todo Martin document */
  bool event_ready();

  /** @todo Martin document */
  EventNMX get();

private:
  std::multimap<uint64_t, Eventlet> backlog_; /**< @todo Martin document */
  std::vector<uint32_t> data;
};
