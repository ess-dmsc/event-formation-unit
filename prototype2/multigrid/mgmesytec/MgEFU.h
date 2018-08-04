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
#include <multigrid/mgmesytec/Hit.h>
#include <multigrid/mgmesytec/MgSeqGeometry.h>

// \todo rescale and thresholds per channel

class MgEFU {
public:
  MgEFU() = default;
  virtual ~MgEFU() = default;

  inline size_t ingest(const std::vector<MGHit> &hits) {
    reset();

    size_t ret{0};
    for (const auto &h : hits) {
      // \todo filter out external trigger non-events
      if (h.external_trigger)
        continue;
      if (this->ingest(h)) {
        ret++;
      }
    }
    return ret;
  }

  virtual void reset() = 0;
  virtual bool ingest(const MGHit& hit) = 0;
  virtual bool event_good() const = 0;

  MgSeqGeometry mappings;
  std::shared_ptr<Hists> hists;

  virtual uint32_t x() const = 0;
  virtual uint32_t y() const = 0;
  virtual uint32_t z() const = 0;
};
