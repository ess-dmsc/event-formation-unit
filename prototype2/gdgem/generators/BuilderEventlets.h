/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from h5 data
 */

#pragma once

#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/nmx/EventletFile.h>

class BuilderEventlets : public AbstractBuilder {
public:
  BuilderEventlets(std::string dump_dir, bool dump_csv, bool dump_h5);

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size) override;

private:
  static constexpr size_t psize{sizeof(Eventlet)};

  std::shared_ptr<EventletFile> eventlet_file_;
  std::vector<Eventlet> converted_data;
};
