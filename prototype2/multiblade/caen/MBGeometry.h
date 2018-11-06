/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// /brief Multiblade geometry
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <stdio.h>

/// \brief geometry for both MB16 and MB18 in Estia and Freia
/// configurations
class MBGeometry {
public:
  MBGeometry(uint16_t cassettes, uint16_t wires, uint16_t strips)
     : NCassettes(cassettes), NWires(wires), NStrips(strips) {
       setConfigurationFreia();
       MaxWireCh = NWires - 1;
       MaxStripCh = NWires + NStrips - 1;
     };

  void setConfigurationFreia() {
    Freia = true;
    yMultiplier = NStrips;
    MaxX = NStrips - 1;
    MaxY = NCassettes * NWires - 1;

  }
  void setConfigurationEstia() {
    Freia = false;
    yMultiplier = NCassettes * NWires;
    MaxX = NCassettes * NWires - 1;
    MaxY = NStrips - 1;
  }

  bool isFreia() { return Freia; }
  bool isEstia() { return not isFreia(); }

  void setDetectorMB18() { MB18 = true; }
  void setDetectorMB16() { MB18 = false; }
  bool isDetectorMB18() { return MB18; }
  bool isDetectorMB16() { return not isDetectorMB18(); }


  uint32_t getPixel(uint16_t cassette, uint16_t localx, uint16_t localy) {
    uint16_t globalx, globaly;
    if (Freia) {
      globalx = localx;
      globaly = cassette * NWires + localy;
    } else {
      globalx = cassette * NWires + localx;
      globaly = localy;
    }
    return getPixel(globalx, globaly);
  }

  uint32_t getPixel(uint16_t x, uint16_t y) {
    if ( (x > MaxX) or (y > MaxY) ) {
      return 0;
    }
    return y * yMultiplier + x + 1;
  }

  // Wires have channels from 0 to NWires -1
  bool isWire(uint16_t channel) {
    return (channel <= MaxWireCh);
  }

  // Strips have channels from NWires to NWires + NStrips - 1
  bool isStrip(uint16_t channel) {
    return ( (not isWire(channel)) and (channel <= MaxStripCh) );
  }

  int getPlane(uint16_t channel) {
    return isStrip(channel) ^ Freia; // 0 is x, 1 is y
  }

  uint32_t getx(uint16_t cassette, uint16_t ch) {
    if (MB18) {
      // Swap odd even
      ch = ch ^ 1;
    }

    if (Freia) {
      return ch - NWires; // or MaxWireCh + 1, same thing
    } else { // Estia
      if (MB18) {
        return cassette * NWires + 31 - ch;
      } else { // MB16
        return cassette * NWires + ch;
      }
    }
  }

  uint32_t gety(uint16_t cassette, uint16_t ch) {
      if (MB18) {
      // Swap odd even
      ch = ch ^ 1;
    }

    if (Freia) {
      if (MB18) {
        return cassette * NWires + ch;
      } else { // MB16
        return cassette * NWires + (31 - ch);
      }
    } else { // Estia
      return ch - NWires; // or MaxWireCh + 1, same thing
    }
  }


  private:
    bool Freia{true}; // implicit estia == false
    bool MB18{true}; // implicit MB16 when mb18 == false;

    uint16_t NCassettes;
    uint16_t NWires; // per cassette
    uint16_t MaxWireCh;
    uint16_t NStrips; // per cassette
    uint16_t MaxStripCh;

    uint16_t yMultiplier;
    uint16_t MaxX;
    uint16_t MaxY;
};
