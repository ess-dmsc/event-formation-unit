/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <memory>
#include <common/Hists.h>
#include <common/ReadoutSerializer.h>
#include <multigrid/mgmesytec/Hit.h>
#include <multigrid/mgmesytec/SequoiaGeometry.h>

namespace Multigrid {

class Efu {
public:
  SequoiaGeometry mappings;
  std::shared_ptr<Hists> hists;
  std::shared_ptr<ReadoutSerializer> raw1;
  std::shared_ptr<ReadoutSerializer> raw2;

  virtual size_t ingest(const std::vector<Hit> &hits) = 0;
  virtual bool event_good() const = 0;

  virtual uint32_t x() const = 0;
  virtual uint32_t y() const = 0;
  virtual uint32_t z() const = 0;
  virtual uint64_t time() const = 0;
};

}
