#pragma once

#include <gdgem/clustering/HitContainer.h>
#include <gdgem/srs/SRSTime.h>

class HitsQueue {
public:
  HitsQueue(SRSTime Time, double maxTimeGap);
  void store(uint8_t plane, uint16_t strip, uint16_t adc, double chipTime);

  // TODO: add a flush flag here
  void sort_and_correct();
  void subsequent_trigger(bool);

  const HitContainer& hits() const;

private:
  // tripple buffer

  // TODO: trigger_timestamp must be added

  HitContainer hitsOld;
  HitContainer hitsNew;
  HitContainer hitsOut;

  SRSTime pTime;
  double pMaxTimeGap {200};
  bool subsequent_trigger_{false};

  void correct_trigger_data();
};
