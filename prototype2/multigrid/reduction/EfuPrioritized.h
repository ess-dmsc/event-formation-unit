/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <multigrid/reduction/Efu.h>
#include <limits>

namespace Multigrid {

class EfuPrioritized : public Efu {
public:
  EfuPrioritized() = default;
  ~EfuPrioritized() = default;

  size_t ingest(const std::vector<Readout>& hits) override;

  void reset();
  bool event_good() const override;

  uint32_t x() const override;
  uint32_t y() const override;
  uint32_t z() const override;
  uint64_t time() const override;

private:
  uint64_t xmass {0};
  uint64_t ymass {0};
  uint64_t zmass {0};

  uint64_t xsum {0};
  uint64_t ysum {0};
  uint64_t zsum {0};

  uint64_t time_ {0};
};

}