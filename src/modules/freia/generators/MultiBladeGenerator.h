// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts for multi-blade based setups
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <common/readout/vmm3/VMM3Parser.h>

namespace Freia {

using VMM3Data = ESSReadout::VMM3Parser::VMM3Data;

class MultiBladeGenerator : public ReadoutGeneratorBase {
  using Image = std::vector<std::pair<uint8_t, uint8_t>>;
  using Images = std::vector<const Image *>;

 public:
  MultiBladeGenerator();

  struct {
    std::string Detector{"Freia"};

    // Masks are used to restrict fibers, FENs, and VMMs
    uint8_t  FiberVals{4};            // Fibers 0 - 4
    uint32_t FiberMask{0x00ffffff};

    uint8_t  FENVals{2};              // FENs   0 - 2
    uint16_t FENMask{0xffff};

    uint8_t  VMMVals{2};              // VMMs   0 - 2
    uint16_t VMMMask{0xff};
  } MultiBladeSettings;

  ///
  /// \brief Intercept the main function to set the detector type
  void main();

 protected:
  ///
  /// \brief Generate readout data and store these in the Buffer container
  ///
  void generateData() override;

  ///
  /// \brief For a given readout index, return a pointer to the readout buffer
  /// of the corresponding VMM3Data data
  ///
  /// \param Index The readout index
  ///
  /// \return A pointer to readout data
  ///
  VMM3Data *getReadoutDataPtr(size_t Index);

  /// \brief Bitmap images used as neutron masks
  Images mImages;

  ///
  const uint32_t mTimeToFirstReadout{1000};
};

} // namespace Freia

// GCOVR_EXCL_STOP
