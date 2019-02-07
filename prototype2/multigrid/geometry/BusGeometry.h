/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/Filter.h>

#include <cinttypes>
#include <string>
#include <sstream>

#include <limits>
#include <vector>

namespace Multigrid {

class BusGeometry {
protected:
  static inline void swap(uint16_t &channel) {
    if (channel % 2 == 0) {
      channel += 1;
    } else {
      channel -= 1;
    }
  }

  uint16_t max_channel_{120};
  uint16_t max_wire_{80};
  uint16_t max_z_{20};

  bool flipped_x_{false};
  bool flipped_z_{false};

  bool swap_wires_{false};
  bool swap_grids_{false};

  std::vector<Filter> wire_filters_;
  std::vector<Filter> grid_filters_;

public:

  inline uint16_t rescale_wire(uint16_t wire, uint16_t adc) const {
    if (wire >= wire_filters_.size())
      return adc;
    return wire_filters_[wire].rescale(adc);
  }

  inline uint16_t rescale_grid(uint16_t grid, uint16_t adc) const {
    if (grid >= grid_filters_.size())
      return adc;
    return grid_filters_[grid].rescale(adc);
  }

  inline bool valid_wire(uint16_t wire, uint16_t adc) const {
    if (wire >= wire_filters_.size())
      return true;
    return wire_filters_[wire].valid(adc);
  }

  inline bool valid_grid(uint16_t grid, uint16_t adc) const {
    if (grid >= grid_filters_.size())
      return true;
    return grid_filters_[grid].valid(adc);
  }

  void set_wire_filters(Filter mgf) {
    wire_filters_.resize(max_wire());
    for (auto &f : wire_filters_)
      f = mgf;
  }

  void set_grid_filters(Filter mgf) {
    grid_filters_.resize(max_grid());
    for (auto &f : grid_filters_)
      f = mgf;
  }

  void override_wire_filter(uint16_t n, Filter mgf) {
    if (wire_filters_.size() <= n)
      wire_filters_.resize(n + 1);
    wire_filters_[n] = mgf;
  }

  void override_grid_filter(uint16_t n, Filter mgf) {
    if (grid_filters_.size() <= n)
      grid_filters_.resize(n + 1);
    grid_filters_[n] = mgf;
  }

  inline void swap_wires(bool s) {
    swap_wires_ = s;
  }

  inline void swap_grids(bool s) {
    swap_grids_ = s;
  }

  inline void max_wire(uint16_t g) {
    max_wire_ = g;
  }

  inline void max_channel(uint16_t g) {
    max_channel_ = g;
  }

  inline void max_z(uint16_t w) {
    max_z_ = w;
  }

  inline void flipped_x(bool f) {
    flipped_x_ = f;
  }

  inline void flipped_z(bool f) {
    flipped_z_ = f;
  }

  inline bool swap_wires() const {
    return swap_wires_;
  }

  inline bool swap_grids() const {
    return swap_grids_;
  }

  inline uint16_t max_channel() const {
    return max_channel_;
  }

  inline uint16_t max_wire() const {
    return max_wire_;
  }

  inline uint16_t max_grid() const {
    return max_channel_ - max_wire_;
  }

  inline bool flipped_x() const {
    return flipped_x_;
  }

  inline bool flipped_z() const {
    return flipped_z_;
  }

  inline uint32_t max_x() const {
    return max_wire_ / max_z_;
  }

  inline uint32_t max_y() const {
    return max_grid();
  }

  inline uint16_t max_z() const {
    return max_z_;
  }

  /** @brief identifies which channels are wires, from drawing by Anton */
  inline bool isWire(uint16_t channel) const {
    return (channel < max_wire_);
  }

  /** @brief identifies which channels are grids, from drawing by Anton */
  inline bool isGrid(uint16_t channel) const {
    return (channel >= max_wire_) && (channel < max_channel_);
  }

  /** @brief returns wire */
  inline uint16_t wire(uint16_t channel) const {
    if (swap_wires_) {
      swap(channel);
    }
    return channel;
  }

  /** @brief returns grid */
  inline uint16_t grid(uint16_t channel) const {
    if (swap_grids_) {
      swap(channel);
    }
    return channel - max_wire_;
  }

  inline uint32_t x_from_wire(uint16_t w) const {
    if (flipped_x_) {
      return (max_wire_ / max_z_) - uint16_t(1) - w / max_z_;
    } else {
      return w / max_z_;
    }
  }

  /** @brief return the x coordinate of the detector */
  virtual uint32_t x(uint16_t channel) const {
    return x_from_wire(this->wire(channel));
  }

  inline uint32_t y_from_grid(uint16_t g) const {
    return g;
  }

  /** @brief return the y coordinate of the detector */
  inline uint32_t y(uint16_t channel) const {
    return y_from_grid(grid(channel));
  }

  inline uint32_t z_from_wire(uint16_t w) const {
    if (flipped_z_) {
      return (max_z_ - uint16_t(1)) - w % max_z_;
    } else {
      return w % max_z_;
    }
  }

  /** @brief return the z coordinate of the detector */
  virtual uint32_t z(uint16_t channel) const {
    return z_from_wire(wire(channel));
  }

  std::string debug(std::string prefix = "") const {
    std::stringstream ss;

    ss << prefix << "wires=chan[0," << (max_wire_ - 1) << "] ";
    if (swap_wires_)
      ss << "(swapped)";
    ss << "\n";

    ss << prefix << "grids=chan[" << max_wire_ << "," << (max_channel_ - 1) << "] ";
    if (swap_grids_)
      ss << "(swapped)";
    ss << "\n";

    ss << prefix << "size [" << max_x() << "," << max_y() << "," << max_z() << "]\n";
    if (flipped_x_)
      ss << prefix << "(flipped in X)\n";
    if (flipped_z_)
      ss << prefix << "(flipped in Z)\n";

    std::stringstream wfilters;
    bool validwf{false};
    for (size_t i = 0; i < wire_filters_.size(); i++) {
      const auto &f = wire_filters_[i];
      if (!f.trivial()) {
        wfilters << prefix << "  [" << i << "]  " << f.debug() << "\n";
        validwf = true;
      }
    }
    if (validwf) {
      ss << prefix << "Wire filters:\n" << wfilters.str();
    }

    std::stringstream gfilters;
    bool validgf{false};
    for (size_t i = 0; i < grid_filters_.size(); i++) {
      const auto &f = grid_filters_[i];
      if (!f.trivial()) {
        gfilters << prefix << "  [" << i << "]  " << f.debug() << "\n";
        validgf = true;
      }
    }
    if (validgf) {
      ss << prefix << "Grid filters:\n" << gfilters.str();
    }

    return ss.str();
  }

};

inline void from_json(const nlohmann::json &j, BusGeometry &g) {
  g.max_channel(j["max_channel"]);
  g.max_wire(j["max_wire"]);
  g.max_z(j["max_z"]);

  if (j.count("swap_wires"))
    g.swap_wires(j["swap_wires"]);
  if (j.count("swap_grids"))
    g.swap_grids(j["swap_grids"]);
  if (j.count("flipped_x"))
    g.flipped_x(j["flipped_x"]);
  if (j.count("flipped_z"))
    g.flipped_z(j["flipped_z"]);

  if (j.count("wire_filters")) {
    auto wf = j["wire_filters"];
    if (wf.count("blanket"))
      g.set_wire_filters(wf["blanket"]);
    if (wf.count("exceptions")) {
      auto wfe = wf["exceptions"];
      for (unsigned int j = 0; j < wfe.size(); j++) {
        g.override_wire_filter(wfe[j]["idx"], wfe[j]);
      }
    }
  }

  if (j.count("grid_filters")) {
    auto gf = j["grid_filters"];
    if (gf.count("blanket"))
      g.set_grid_filters(gf["blanket"]);
    if (gf.count("exceptions")) {
      auto gfe = gf["exceptions"];
      for (unsigned int j = 0; j < gfe.size(); j++) {
        g.override_grid_filter(gfe[j]["idx"], gfe[j]);
      }
    }
  }

}

}

