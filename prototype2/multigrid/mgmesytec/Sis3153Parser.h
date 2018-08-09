/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/Buffer.h>
#include <vector>

namespace Multigrid {

class Sis3153Parser {
public:
  /// \brief parse a binary payload, return number readout buffers within
  ///        fills the buffer member variable with references to sub-buffers
  size_t parse(const Buffer<uint8_t> &buffer);

  std::vector<Buffer<uint32_t>> buffers;
};

}