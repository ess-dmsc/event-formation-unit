#pragma once

#include <gdgem/dg_impl/HitContainer.h>
#include <gdgem/vmm2srs/SRSTime.h>

class HitsQueue {
public:
  HitsQueue(SRSTime Time, double maxTimeGap);
  void store(uint16_t strip, uint16_t adc, double chipTime);
  void sort_and_correct();
  void subsequent_trigger(bool);

  const HitContainer& hits() const;

private:
  // tripple buffer

  HitContainer hitsOld;
  HitContainer hitsNew;
  HitContainer hitsOut;

  SRSTime pTime;
  double pMaxTimeGap {200};
  bool subsequent_trigger_{false};

  void correct_trigger_data();
};
