// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX geometry class
/// Based on TREX ICD documen
/// thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <trex/geometry/Geometry.h>

namespace Trex {

class LETGeometry : public Trex::Geometry {
public:
  bool isWire(uint8_t LocalHybridID) override {
    if (LocalHybridID == 1) {
      return true;
    } else {
      return false;
    }
  }

  bool isGrid(uint8_t LocalHybridID) override { return !isWire(LocalHybridID); }

  uint16_t xAndzCoord(uint8_t RingID, uint8_t FENID, uint8_t HybridID,
                      uint8_t VMMID, uint8_t Channel, uint16_t XOffset,
                      bool Rotated) override;
  uint16_t yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel,
                  uint16_t YOffset, bool Rotated, bool Short) override;

protected:
  bool validGridMapping(uint8_t HybridID, uint8_t VMMID, uint8_t Channel,
                        bool Short) override;
  bool validWireMapping(uint8_t HybridID, uint8_t VMMID,
                        uint8_t Channel) override;
};

} // namespace Trex
