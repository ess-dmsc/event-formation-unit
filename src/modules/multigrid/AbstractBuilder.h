// Copyright (C) 2016-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Interface to parse Multi-Grid readout data
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/memory/Buffer.h>
#include <common/reduction/HitVector.h>

namespace Multigrid {

class AbstractBuilder {
public:
  virtual void parse(Buffer<uint8_t> buffer) = 0;
  virtual std::string debug() const = 0;
  virtual ~AbstractBuilder() = default;

  HitVector ConvertedData;

  size_t stats_readouts_total {0};
  size_t stats_discarded_bytes {0};
  size_t stats_trigger_count {0};
  size_t stats_bus_glitch_rejects {0};
  size_t stats_readout_filter_rejects {0};
  size_t stats_digital_geom_errors {0};
};

}
