/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <gdgem/nmx/AbstractAnalyzer.h>

namespace Gem {

// \todo rename this
class MGAnalyzer : public AbstractAnalyzer {
public:
  MultiDimResult analyze(Event&) const override;
  std::string debug() const override;

  void weighted(bool w);
  bool weighted() const;

  void max_grid(uint16_t g);
  void max_wire(uint16_t w);
  void max_z(uint16_t w);
  void flipped_x(bool f);
  void flipped_z(bool f);
  bool flipped_x() const;
  bool flipped_z() const;

  uint16_t max_wire() const;
  uint16_t max_grid() const;

  uint32_t max_x() const;
  uint32_t max_y() const;
  uint16_t max_z() const;

  virtual uint32_t x_from_wire(uint16_t w) const;
  virtual uint32_t y_from_grid(uint16_t g) const;
  virtual uint32_t z_from_wire(uint16_t w) const;

  mutable size_t stats_used_hits {0};

private:
  bool weighted_{true};

  uint16_t max_grid_{16};
  uint16_t max_wire_{80};
  uint16_t max_z_{20};

  bool flipped_x_{false};
  bool flipped_z_{false};
};

}
