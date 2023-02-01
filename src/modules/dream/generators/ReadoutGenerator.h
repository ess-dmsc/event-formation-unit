// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial DREAM readouts with specific pattern
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <dream/readout/DataParser.h>
//#include <common/testutils/DataFuzzer.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

namespace Dream {
class DreamReadoutGenerator : public ReadoutGeneratorBase {
public:
  using ReadoutGeneratorBase::ReadoutGeneratorBase;

  // Physical Ring and FEN ids for detector elements
  // Should match the json config file
  uint8_t BWES6RingId[11] = {0, 0, 0, 0, 0,  0, 2, 2, 2, 2, 2};
  uint8_t BWES6FENId[11]  = {0, 2, 4, 6, 8, 10, 0, 2, 4, 6, 8};

  uint8_t FWES6RingId[5] = {4, 4, 4, 4, 4};
  uint8_t FWES6FENId[5]  = {0, 2, 4, 6, 8};

  uint8_t MNTLRingId[30] = {6,6,6,6,6,6,  8,8,8,8,8,8,  10,10,10,10,10,10,  12,12,12,12,12,12,12,12,12,12,12,12};
  uint8_t MNTLFENId[30]  = {0,1,2,3,4,5,  0,1,2,3,4,5,   0, 1, 2, 3, 4, 5,   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11};

  uint8_t HRRingId[17]   = {14,14,14,14,14,14,14,14, 16,16,16,16,16,16,16,16,16};
  uint8_t HRFENId[17]    = { 0, 1, 2, 3, 4, 5, 6, 7,  0, 1, 2, 3, 4, 5, 6, 7, 8};

  uint8_t SANSRingId[18] = {18,18,18,18,18,18,18,18,18, 20,20,20,20,20,20,20,20,20};
  uint8_t SANSFENId[18]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8,  0, 1, 2, 3, 4, 5, 6, 7, 8};

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};

  ///\brief
  void getRandomReadout(DataParser::DreamReadout & DR);

};
} // namespace Dream
// GCOVR_EXCL_STOP
