/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from h5 data
 */

#pragma once

#include <gdgem/nmx/AbstractEventletBuilder.h>
#include <vector>

class BuilderH5 : public AbstractBuilder {
public:
  BuilderH5(std::string dump_dir, bool dump_csv, bool dump_h5);

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size, Clusterer &clusterer,
                             NMXHists &hists) override;

private:
  size_t psize{sizeof(uint32_t) * 4};
  std::vector<uint32_t> data;

  Eventlet make_eventlet();
};
