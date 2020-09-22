/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for NMX plane ID & strip mappings
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Readout.h>
#include <common/reduction/Hit.h>
#include <vector>
#include <list>

namespace Gem {

class SRSMappings {
public:
  static constexpr size_t MaxChannelsInVMM {64};
  static constexpr size_t MaxChipsInFEC {16};

  //// \brief define mappings for sequence of chips in one plane
  /// \param planeID ID of plane (edge of panel) being defined
  /// \param chips list of (FEC, VMM) pairs in the order of increasing strip
  /// number
  void define_plane(uint8_t planeID,
                    std::list<std::pair<uint16_t, uint16_t>> chips);

  /// \brief define channel mappings for one chip
  /// \param fecID ID of FEC
  /// \param vmmID ID of chip
  /// \param planeID ID of plane (edge of panel)
  /// \param strip_offset starting strip for this chip's channels
  void set_mapping(uint16_t fecID, uint16_t vmmID, uint8_t planeID,
                   uint16_t strip_offset);

  void set_mapping(uint16_t fecID, uint16_t vmmID, uint16_t channel,
                   uint8_t plane, uint16_t coord);

  uint16_t get_strip(const Readout &readout) const;

  uint8_t get_plane(const Readout &readout) const;

  /// \brief prints out configuration
  std::string debug() const;

private:
  struct MappingResult
  {
    uint8_t plane {Hit::InvalidPlane};
    uint16_t coordinate {Hit::InvalidCoord};
  };

  using ChipMappings = std::vector<MappingResult>;
  using FecMappings = std::vector<ChipMappings>;

  std::vector<FecMappings> mappings_;
};

}
