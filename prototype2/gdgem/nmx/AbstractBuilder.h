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
    uint32_t parser_lost_frames{0}; /// added in ParserVMM3
    uint32_t parser_bad_frames{0};  /// added in ParserVMM3
    uint32_t parser_good_frames{0}; /// added in ParserVMM3
    uint32_t parser_error_bytes{0};
    uint32_t parser_readouts{0};
    uint32_t geom_errors{0};
    uint32_t adc_rejects{0};
  };

  AbstractBuilder() = default;

  virtual ~AbstractBuilder() = default;

  /// \todo someone document
  // \todo use Buffer<char>
  virtual void process_buffer(char *buf, size_t size) = 0;

  HitContainer hit_buffer_x;
  HitContainer hit_buffer_y;

  ResultStats stats;
};

}
