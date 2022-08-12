/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/memory/Buffer.h>
#include <vector>

namespace Multigrid {

class Sis3153Parser {
public:
  Sis3153Parser();

  /// \brief parse a binary payload, return number of bytes discarded
  size_t parse(Buffer<uint8_t> buffer);

  /// \brief refrences to buffers within the Sis buffer
  std::vector<Buffer<uint32_t>> buffers;
};

} // namespace Multigrid