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
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  /// \brief parse a binary payload buffer, return number of data element
  error parse(const Buffer &buffer);

  std::vector<Buffer> buffers;
};

}