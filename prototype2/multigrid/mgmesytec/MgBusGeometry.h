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

public:
  
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

    return ss.str();
  }

};
