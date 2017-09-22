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
  memset(x_adc_hist, 0, sizeof(x_adc_hist));
  memset(y_adc_hist, 0, sizeof(y_adc_hist));
  memset(cluster_adc_hist, 0, sizeof(cluster_adc_hist));
  xyhist_elems = 0;
}

void NMXHists::bin(const Eventlet& e)
{
  if (e.plane_id == 0)
  {
    x_strips_hist[e.strip]++;
    x_adc_hist[e.adc]++;
  }
  else if (e.plane_id == 1)
  {
    y_strips_hist[e.strip]++;
    y_adc_hist[e.adc]++;
  }
  else
    return;
  xyhist_elems++;
}

void NMXHists::bin(const EventNMX& e)
{
  uint32_t sum = e.x.integral + e.y.integral;
  if (!sum)
    return;
  cluster_adc_hist[static_cast<uint16_t>(sum >> downshift_)]++;
  xyhist_elems++;
}
