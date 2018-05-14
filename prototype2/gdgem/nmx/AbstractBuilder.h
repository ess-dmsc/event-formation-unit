/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Abstract class for creating NMX eventlets
 */

#pragma once

#include <gdgem/clustering/AbstractClusterer.h>
#include <common/DataSave.h>
#include <memory>

class AbstractBuilder {
public:
  struct ResultStats {
    ResultStats() {}
    ResultStats(uint32_t ev, uint32_t err, uint32_t geom_err)
        : valid_eventlets(ev), error_bytes(err), geom_errors(geom_err) {}

    uint32_t valid_eventlets{0};
    uint32_t error_bytes{0};
    uint32_t geom_errors{0};
  };

  AbstractBuilder(std::string dump_dir, bool dump_csv, bool dump_h5);

  AbstractBuilder(std::shared_ptr<AbstractClusterer> x,
                  std::shared_ptr<AbstractClusterer> y,
                  std::string dump_dir, bool dump_csv, bool dump_h5);


  virtual ~AbstractBuilder() {}

  /** @todo Martin document */
  virtual ResultStats process_buffer(char *buf, size_t size) = 0;

  std::shared_ptr<AbstractClusterer> clusterer_y;
  std::shared_ptr<AbstractClusterer> clusterer_x;

protected:
  bool dump_csv_{false};
  bool dump_h5_{false};

  // CSV
  std::shared_ptr<DataSave> vmmsave;

  static std::string time_str();
};
