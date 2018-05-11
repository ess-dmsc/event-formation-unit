/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for creating NMX eventlets from h5 data
 */

#pragma once

#include <gdgem/nmx/AbstractEventletBuilder.h>
#include <vector>

class BuilderEventlets : public AbstractBuilder {
public:
  BuilderEventlets(std::string dump_dir, bool dump_csv, bool dump_h5);

  /** @todo Martin document */
  ResultStats process_buffer(char *buf, size_t size, Clusterer &clusterer,
                             NMXHists &hists) override;

private:
  static constexpr size_t psize{sizeof(Eventlet)};

  hdf5::datatype::Datatype dtype_;
  hdf5::node::Dataset dataset_;
  hdf5::dataspace::Hyperslab slab_{{0}, {9000 / psize}};
  std::vector<Eventlet> converted_data;

  void setup_h5(std::string dump_dir);
  void write_h5();
};
