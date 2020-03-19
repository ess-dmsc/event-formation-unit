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
    uint32_t ParserFrameSeqErrors{0}; /// added in ParserVMM3
    uint32_t ParserFramecounterOverflows{0}; /// added in ParserVMM3
    uint32_t ParserTimestampLostErrors{0}; /// added in ParserVMM3
    uint32_t ParserTimestampSeqErrors{0}; /// added in ParserVMM3
    uint32_t ParserTimestampOverflows{0}; /// added in ParserVMM3
    uint32_t ParserBadFrames{0};  /// added in ParserVMM3
    uint32_t ParserGoodFrames{0}; /// added in ParserVMM3
    uint32_t ParserErrorBytes{0};
    uint32_t ParserReadouts{0};
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
