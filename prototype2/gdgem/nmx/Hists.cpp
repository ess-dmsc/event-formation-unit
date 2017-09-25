/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hists.h>
#include <assert.h>
#include <string.h>

NMXHists::NMXHists()
{
  clear();
}

void NMXHists::clear() {
  memset(xyhist, 0, sizeof(xyhist));
  xyhist_elems = 0;
}

void NMXHists::bin_one(uint16_t plane_id, uint16_t strip)
{
  assert(plane_id == 0 || plane_id == 1);
  assert(strip <= 1500);
  xyhist[plane_id][strip]++;
  xyhist_elems++;
}
