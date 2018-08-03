/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <cinttypes>
#include <string>
#include <sstream>

#include <limits>
#include <vector>

struct MgFilter {
  uint16_t minimum {0};
  uint16_t maximum {std::numeric_limits<uint16_t>::max()};
  double rescale {1.0};

  bool non_trivial() const {
    return ((minimum != 0) || (maximum != std::numeric_limits<uint16_t>::max()) ||
        (rescale != 1.0));
  }

  std::string debug() const {
    std::stringstream ss;
    if (minimum != 0)
      ss << "min=" << minimum << "  ";
    if (maximum != std::numeric_limits<uint16_t>::max())
      ss << "max=" << maximum << "  ";
    if (rescale != 1.0)
      ss << "rescale=" << rescale << "  ";
    return ss.str();
  }
};

class MgBusGeometry {
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
  uint16_t max_z_ {20};

  bool flipped_x_ {false};
  bool flipped_z_ {false};

  bool swap_wires_{false};
  bool swap_grids_{false};

  std::vector<MgFilter> wire_filters_;
  std::vector<MgFilter> grid_filters_;

public:

  inline uint16_t rescale_wire(uint16_t wire, uint16_t adc) const
  {
    if (wire >= wire_filters_.size())
      return adc;
    return static_cast<uint16_t>(adc * wire_filters_.at(wire).rescale);
  }

  inline uint16_t rescale_grid(uint16_t grid, uint16_t adc) const
  {
    if (grid >= grid_filters_.size())
      return adc;
    return static_cast<uint16_t>(adc * grid_filters_.at(grid).rescale);
  }

  inline bool valid_wire(uint16_t wire, uint16_t adc) const
  {
    if (wire >= wire_filters_.size())
      return true;
    const auto& f = wire_filters_.at(wire);
    return ((f.minimum <= adc) && (adc <= f.maximum));
  }

  inline bool valid_grid(uint16_t grid, uint16_t adc) const
  {
    if (grid >= grid_filters_.size())
      return true;
    const auto& f = grid_filters_.at(grid);
    return ((f.minimum <= adc) && (adc <= f.maximum));
  }

  void set_wire_filters(MgFilter mgf) {
    wire_filters_.resize(max_wire());
    for (auto& f : wire_filters_)
      f = mgf;
  }

  void set_grid_filters(MgFilter mgf) {
    grid_filters_.resize(max_grid());
    for (auto& f : grid_filters_)
      f = mgf;
  }

  void override_wire_filter(uint16_t n, MgFilter mgf) {
    if (wire_filters_.size() <= n)
      wire_filters_.resize(n+1);
    wire_filters_[n] = mgf;
  }

  void override_grid_filter(uint16_t n, MgFilter mgf) {
    if (grid_filters_.size() <= n)
      grid_filters_.resize(n+1);
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

  /** @brief return the x coordinate of the detector */
  inline uint32_t x(uint16_t channel) const {
    if (flipped_x_) {
      return (max_wire_ / max_z_) - uint16_t(1) - wire(channel) / max_z_;
    } else {
      return wire(channel) / max_z_;
    }
  }

  /** @brief return the y coordinate of the detector */
  inline uint32_t y(uint16_t channel) const {
    return grid(channel);
  }

  /** @brief return the z coordinate of the detector */
  inline uint32_t z(uint16_t channel) const {
    if (flipped_z_) {
      return (max_z_ - uint16_t(1)) - wire(channel) % max_z_;
    } else {
      return wire(channel) % max_z_;
    }
  }

  std::string debug(std::string prefix) const {
    std::stringstream ss;

    ss << prefix <<  "wires=chan[0," << (max_wire_-1) << "] ";
    if (swap_wires_)
      ss << "(swapped)";
    ss << "\n";

    ss << prefix <<  "grids=chan[" << max_wire_ << "," << (max_channel_-1) << "] ";
    if (swap_grids_)
      ss << "(swapped)";
    ss << "\n";

    ss << prefix << "size [" << max_x() << "," << max_y() << "," << max_z() << "]\n";
    if (flipped_x_)
      ss << prefix << "(flipped in X)\n";
    if (flipped_z_)
      ss << prefix << "(flipped in Z)\n";

    std::stringstream wfilters;
    bool validwf {false};
    for (size_t i=0; i < wire_filters_.size(); i++) {
      const auto& f = wire_filters_.at(i);
      if (f.non_trivial()) {
        wfilters << prefix << "  [" << i  << "]  " <<  f.debug() << "\n";
        validwf = true;
      }
    }
    if (validwf) {
      ss << prefix << "Wire filters:\n" << wfilters.str();
    }

    std::stringstream gfilters;
    bool validgf {false};
    for (size_t i=0; i < grid_filters_.size(); i++) {
      const auto& f = grid_filters_.at(i);
      if (f.non_trivial()) {
        gfilters << prefix << "  [" << i  << "]  " <<  f.debug() << "\n";
        validgf = true;
      }
    }
    if (validgf) {
      ss << prefix << "Grid filters:\n" << gfilters.str();
    }

    return ss.str();
  }

};
