/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/clustering/HitContainer.h>
#include <gdgem/srs/SRSTime.h>

namespace Gem {

class HitsQueue {
public:
  HitsQueue(SRSTime Time, double maxTimeGap);
  void store(uint8_t plane, uint16_t strip, uint16_t adc,
             double chipTime, double trigger_time = 0);

  /// \todo add a flush flag here
  void sort_and_correct();

  const HitContainer &hits() const;

private:
  // tripple buffer

  HitContainer hitsOld;
  HitContainer hitsNew;
  HitContainer hitsOut;

  SRSTime pTime;
  double pMaxTimeGap{200};
};

}
