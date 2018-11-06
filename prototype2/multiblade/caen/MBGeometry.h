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

  void setMB18() { MB18 = true; }
  void setMB16() { MB18 = false; }
  bool isMB18() { return MB18; }
  bool isMB16() { return not isMB18(); }


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


  uint32_t getx(uint16_t cassette, uint16_t wire_ch, uint16_t strip_ch) {
    if ( (not isWire(wire_ch)) or (not isStrip(strip_ch)) ) {
      return 0xffffffff; // will produce invalid geometry
    }

    if (MB18) {
      // Swap odd even
      wire_ch  = wire_ch  ^ 1;
      strip_ch = strip_ch ^ 1;
    }

    if (Freia) {
      return strip_ch - NWires; // or MaxWireCh + 1, same thing
    } else { // Estia
      if (MB18) {
        return cassette * NWires + 31 - wire_ch;
      } else { // MB16
        return cassette * NWires + wire_ch;
      }
    }
  }

  uint32_t gety(uint16_t cassette, uint16_t wire_ch, uint16_t strip_ch) {
    if ( (not isWire(wire_ch)) or (not isStrip(strip_ch)) ) {
      return 0xffffffff; // will produce invalid geometry
    }

    if (MB18) {
      // Swap odd even
      wire_ch  = wire_ch  ^ 1;
      strip_ch = strip_ch ^ 1;
    }

    if (Freia) {
      if (MB18) {
        return cassette * NWires + wire_ch;
      } else { // MB16
        return cassette * NWires + (31 - wire_ch);
      }
    } else { // Estia
      return strip_ch - NWires; // or MaxWireCh + 1, same thing
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
