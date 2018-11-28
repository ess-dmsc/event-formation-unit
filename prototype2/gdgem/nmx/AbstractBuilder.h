/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Abstract class for creating NMX hits
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractClusterer.h>
#include <memory>

namespace Gem {

class AbstractBuilder {
public:
  struct ResultStats {
    ResultStats() = default;
    ResultStats(uint32_t hits, uint32_t err, uint32_t geom_err)
        : valid_hits(hits), error_bytes(err), geom_errors(geom_err) {}

    /// ParserVMM3 returns more stat data
    ResultStats(uint32_t hits, uint32_t err, uint32_t geom_err, uint32_t adc_rej,
                uint32_t lost_frames, uint32_t bad_frames, uint32_t good_frames)
        : valid_hits(hits), error_bytes(err), geom_errors(geom_err),
        adc_rejects(adc_rej), lost_frames(lost_frames),
        bad_frames(bad_frames), good_frames(good_frames) {}

    uint32_t valid_hits{0};
    uint32_t error_bytes{0};
    uint32_t geom_errors{0};
    uint32_t adc_rejects{0};
    uint32_t lost_frames{0}; /// added in ParserVMM3
    uint32_t bad_frames{0};  /// added in ParserVMM3
    uint32_t good_frames{0}; /// added in ParserVMM3
  };

  AbstractBuilder() = default;

  virtual ~AbstractBuilder() = default;

  /// \todo Martin document
  virtual ResultStats process_buffer(char *buf, size_t size) = 0;

  HitContainer hit_buffer_x;
  HitContainer hit_buffer_y;
};

}
