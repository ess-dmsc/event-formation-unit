/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 */

 // @todo very much work in progress

 #pragma once

class MGSEQDetector {
public:
  /** @wbrief identifies which channels are wires, @todo verify */
  bool isWire(int channel) {return (channel < 32);}

  /** @wbrief identifies which channels are grids, @todo verify */
  bool isGrid(int channel) {return (channel >= 32);}
};
