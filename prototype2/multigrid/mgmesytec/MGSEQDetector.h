/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 */

// @todo very much work in progress

#pragma once

class MGSEQDetector {
public:
  /** @brief identifies which channels are wires, from drawing by Anton */
  inline bool isWire(int channel) { return (channel <= 79) && (channel >= 0); }

  /** @brief identifies which channels are grids, from drawing by Anton */
  inline bool isGrid(int channel) { return (channel >= 80) && (channel < 128); }

  /** @brief return the x coordinate of the detector */
  inline int xcoord(int digno, int channel) {
    if (!isWire(channel)) {
      return -1;
    }
    // wire == channel, wires range from 1 - 80
    if (channel <= 64) {
      return (channel - 1) / 16 + digno * 4;
    } else {
      return (channel - 65) / 4 + digno * 4;
    }
  }

  /** @brief return the y coordinate of the detector */
  inline int ycoord(int channel) {
    if (!isGrid(channel)) {
      return -1;
    }

    return channel - 80; // grids range from 1 to 92
  }

  /** @brief return the z coordinate of the detector */
  inline int zcoord(int channel) {
    if (!isWire(channel)) {
      return -1;
    }

    if (channel <= 64) {
      return (channel - 1) % 16;
    } else {
      return (channel - 65) % 4;
    }
  }
};
