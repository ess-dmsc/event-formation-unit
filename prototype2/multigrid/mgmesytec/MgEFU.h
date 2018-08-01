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
#include <multigrid/mgmesytec/MgGeometry.h>

class MgEFU {
public:
  MgEFU() = default;
  virtual ~MgEFU() = default;

  virtual void reset() = 0;
  virtual bool ingest(uint8_t bus, uint16_t channel, uint16_t adc) = 0;
  virtual bool event_good() const = 0;

  std::shared_ptr<MgGeometry> mappings;
  std::shared_ptr<Hists> hists;

  virtual uint32_t x() const = 0;
  virtual uint32_t y() const = 0;
  virtual uint32_t z() const = 0;
};
