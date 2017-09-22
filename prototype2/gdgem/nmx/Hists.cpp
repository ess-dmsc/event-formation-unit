/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hists.h>
#include <string.h>

NMXHists::NMXHists()
{
  clear();
}

void NMXHists::clear() {
  memset(x_strips_hist, 0, sizeof(x_strips_hist));
  memset(y_strips_hist, 0, sizeof(y_strips_hist));
  xyhist_elems = 0;
}

void NMXHists::bin(const Eventlet& e)
{
  if (e.plane_id == 0)
    x_strips_hist[e.strip]++;
  else if (e.plane_id == 1)
    y_strips_hist[e.strip]++;
  else
    return;
  xyhist_elems++;
}
