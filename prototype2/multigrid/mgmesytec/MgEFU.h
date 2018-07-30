/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <limits>
#include <memory>
#include <common/Hists.h>
#include <multigrid/mgmesytec/MgGeometry.h>

class MgEFU {
public:
  MgEFU(std::shared_ptr<MgGeometry> mg_mappings, std::shared_ptr<NMXHists> h);
  ~MgEFU() = default;

  void setWireThreshold(uint16_t low, uint16_t high);
  void setGridThreshold(uint16_t low, uint16_t high);

  void reset_maxima();
  bool ingest(uint8_t bus, uint16_t channel, uint16_t adc);
  bool event_good() const;

  std::shared_ptr<NMXHists> hists;

  uint32_t x;
  uint32_t y;
  uint32_t z;

private:
  uint16_t wireThresholdLo{0};
  uint16_t wireThresholdHi{std::numeric_limits<uint16_t>::max()};
  uint16_t gridThresholdLo{0};
  uint16_t gridThresholdHi{std::numeric_limits<uint16_t>::max()};
  std::shared_ptr<MgGeometry> MgMappings;

  uint16_t GridAdcMax {0};
  uint16_t WireAdcMax {0};

  bool WireGood{false};
  bool GridGood{false};
};
