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

  // Ring and FEN ids for a specified sector (BwEndcap)
  uint8_t S6RingId[11] = {0, 0, 0, 0, 0,  0, 1, 1, 1, 1, 1};
  uint8_t S6FENId[11] =  {0, 2, 4, 6, 8, 10, 0, 2, 4, 6, 8};

protected:
  void generateData() override;
  const uint32_t TimeToFirstReadout{1000};

  ///\brief
  void getRandomReadout(DataParser::DreamReadout & DR);

};
} // namespace Dream
// GCOVR_EXCL_STOP
