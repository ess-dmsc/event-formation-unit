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

  uint16_t getGlobalChannel(uint16_t cassette, uint16_t channel) const {
    if (isWire(channel)) {
      return NWires * cassette + channel;
    } else {
      return NStrips * cassette + (channel - NWires);
    }
  }

  bool isFreia() const { return Freia; }
  bool isEstia() const { return not isFreia(); }
  bool isValidCh(uint16_t ch) const { return ch <= MaxStripCh; }
  bool isWire(uint16_t channel) const { return (channel <= MaxWireCh); }
  bool isStrip(uint16_t channel) const {
    return ( (not isWire(channel)) and (channel <= MaxStripCh) ); }
  bool isDetectorMB18() const { return MB18; }
  bool isDetectorMB16() const { return not isDetectorMB18(); }
  void setDetectorMB18() { MB18 = true; }
  void setDetectorMB16() { MB18 = false; }

  uint32_t getPixel(uint16_t cassette, uint16_t localx, uint16_t localy) const {
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

  uint32_t getPixel(uint16_t x, uint16_t y) const {
    if ( (x > MaxX) or (y > MaxY) ) {
      return 0;
    }
    return y * yMultiplier + x + 1U;
  }

  uint8_t getPlane(uint16_t channel) const {
    return isStrip(channel) ^ Freia; // 0 is x, 1 is y
  }

  uint16_t getx(uint16_t cassette, uint16_t ch) const {
    if (MB18) {
      // Swap odd even
      ch = ch ^ 1;
    }

    if (Freia) {
      return ch - NWires; // or MaxWireCh + 1, same thing
    } else { // Estia
      if (MB18) {
        return cassette * NWires + uint16_t(31) - ch;
      } else { // MB16
        return cassette * NWires + ch;
      }
    }
  }

  uint16_t gety(uint16_t cassette, uint16_t ch) const {
      if (MB18) {
      // Swap odd even
      ch = ch ^ 1;
    }

    if (Freia) {
      if (MB18) {
        return cassette * NWires + ch;
      } else { // MB16
        return cassette * NWires + (uint16_t(31) - ch);
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
