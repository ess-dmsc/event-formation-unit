/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Abstract class for creating NMX hits
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/HitVector.h>

namespace Gem {

class AbstractBuilder {
public:
  struct ResultStats {
    uint32_t parser_frame_seq_errors{0}; /// added in ParserVMM3
    uint32_t parser_framecounter_overflows{0}; /// added in ParserVMM3
    uint32_t parser_timestamp_lost_errors{0}; /// added in ParserVMM3
    uint32_t parser_timestamp_seq_errors{0}; /// added in ParserVMM3
    uint32_t parser_timestamp_overflows{0}; /// added in ParserVMM3
    uint32_t parser_bad_frames{0};  /// added in ParserVMM3
    uint32_t parser_good_frames{0}; /// added in ParserVMM3
    uint32_t parser_error_bytes{0};
    uint32_t parser_readouts{0};
    uint32_t geom_errors{0};
    uint32_t adc_rejects{0};
    uint32_t adc_zero{0};
  };

  AbstractBuilder() = default;

  virtual ~AbstractBuilder() = default;

  /// \todo someone document
  // \todo use Buffer<char>
  virtual void process_buffer(char *buf, size_t size) = 0;

  HitVector hit_buffer_x;
  HitVector hit_buffer_y;

  ResultStats stats;
};

}
