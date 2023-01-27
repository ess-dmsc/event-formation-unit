// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
/// Based on CSPEC ICD documen
/// thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <cspec/geometry/Geometry.h>

namespace Cspec {

class CSPECGeometry : public Cspec::Geometry {
public:
  ///
  ///\brief determines if a hybrid is for a wire, as opposed to a grid
  ///
  ///\param LocalHybridID integer value identifying a hybrid
  ///\return true if the hybrid is for wires
  ///\return false if the hybrid is for grids
  ///
  bool isWire(uint8_t LocalHybridID) override {
    if (LocalHybridID == 0) {
      return true;
    } else {
      return false;
    }
  }

  ///\brief determines if a hybrid is for a grid, as opposed to a wire
  ///
  ///\param LocalHybridID integer value identifying a hybrid
  ///\return true if the hybrid is for grids
  ///\return false if the hybrid is for wires
  bool isGrid(uint8_t LocalHybridID) override { return !isWire(LocalHybridID); }

  ///
  ///\brief Returns a single integer for the x & z coordinate, note that the x
  ///     and z dimensions are squashed into a single dimension for clustering
  ///     and matching for the multigrid technology, as wires are distinct x & z
  ///     identifiers.
  ///
  ///\param RingID integer value identifying what ring a readout is from
  ///\param FENID integer value identifying what FEN a readout is from
  ///\param HybridID integer value identifying what hybrid a readout is from
  ///\param VMMID integer value identifying what VMM a readout is from
  ///\param Channel integer value between 0 and 63 identifying what channel
  ///         a readout is from
  ///\param XOffset integer value identifying the offset in the X direction
  ///\param Rotated boolean identifying whether a vessel is rotated, ie. above
  ///         the beamstop vessels are rotated so the electronics are in the
  ///         correct location.
  ///\return uint16_t
  ///
  uint16_t xAndzCoord(uint8_t RingID, uint8_t FENID, uint8_t HybridID,
                      uint8_t VMMID, uint8_t Channel, uint16_t XOffset,
                      bool Rotated) override;

  ///
  ///\brief returns an integer for the y coordinate
  ///
  ///\param HybridID integer value identifying what hybrid a readout is from
  ///\param VMMID integer value identifying what VMM a readout is from
  ///\param Channel integer value between 0 and 63 identifying what channel
  ///         a readout is from
  ///\param YOffset integer value identifying the offset in the Y direction,
  ///         ie. below the beamstop the vessel is at a different height to
  ///         normal, so doesn't start at 0 in the Y direction.
  ///\param Rotated boolean identifying whether a vessel is rotated, ie. above
  ///         the beamstop vessels are rotated so the electronics are in the
  ///         correct location.
  ///\param Short boolean identifying whether the vessel is a short or standard
  ///         length, ie. around the beamstop shorter vessels are used.
  ///\return uint16_t
  ///
  uint16_t yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel,
                  uint16_t YOffset, bool Rotated, bool Short) override;

protected:
  bool validGridMapping(uint8_t HybridID, uint8_t VMMID, uint8_t Channel,
                        bool Short) override;
  bool validWireMapping(uint8_t HybridID, uint8_t VMMID,
                        uint8_t Channel) override;
};

} // namespace Cspec
