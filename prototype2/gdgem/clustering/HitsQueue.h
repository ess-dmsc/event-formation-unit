/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/clustering/HitContainer.h>
#include <gdgem/srs/SRSTime.h>

class HitsQueue {
private:
  struct HitBuffer
  {
    double trigger_time {0};
    HitContainer buffer;
  };

public:
  HitsQueue(SRSTime Time, double maxTimeGap);
  void store(uint8_t plane, uint16_t strip, uint16_t adc,
             double chipTime, double trigger_time = 0);

  /// \todo add a flush flag here
  void sort_and_correct();
  void subsequent_trigger(bool);

  const HitContainer& hits() const;

private:
  // tripple buffer

  /// \todo trigger_timestamp must be added

  HitBuffer hitsOld;
  HitBuffer hitsNew;
  HitBuffer hitsOut;

  SRSTime pTime;
  double pMaxTimeGap {200};
  bool subsequent_trigger_{false};

  void correct_trigger_data();
};
