/// Copyright (C) 2017 - 2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Abstract class for creating NMX hits
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/clustering/AbstractClusterer.h>
#include <common/DataSave.h>
#include <memory>

class AbstractBuilder {
public:
  struct ResultStats {
    ResultStats() {}
    ResultStats(uint32_t hits, uint32_t err, uint32_t geom_err)
        : valid_hits(hits), error_bytes(err), geom_errors(geom_err) {}

    /// ParserVMM3 returns more stat data
    ResultStats(uint32_t hits, uint32_t err, uint32_t geom_err,
                uint32_t lost_frames, uint32_t bad_frames, uint32_t good_frames)
        : valid_hits(hits), error_bytes(err), geom_errors(geom_err)
        , lost_frames(lost_frames), bad_frames(bad_frames)
        , good_frames(good_frames) {}

    uint32_t valid_hits{0};
    uint32_t error_bytes{0};
    uint32_t geom_errors{0};
    uint32_t lost_frames{0}; /// added in ParserVMM3
    uint32_t bad_frames{0};  /// added in ParserVMM3
    uint32_t good_frames{0}; /// added in ParserVMM3
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
