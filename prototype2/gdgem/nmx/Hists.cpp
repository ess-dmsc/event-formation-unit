/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hists.h>
#include <string.h>

//static
size_t NMXHists::needed_buffer_size()
{
  return NMX_HIST_ELEM_SIZE *
      (NMX_STRIP_HIST_SIZE * 2 +
       NMX_ADC_HIST_SIZE * 3 +
       1 /*bin_width*/ );
}

NMXHists::NMXHists()
{
  clear();
}

void NMXHists::set_cluster_adc_downshift(uint32_t bits)
{
  if (bits > 32)
    bits = 32;
  downshift_ = bits;
}

bool NMXHists::empty() const
{
  return eventlet_count_ || cluster_count_;
}

size_t NMXHists::eventlet_count() const
{
  return eventlet_count_;
}

size_t NMXHists::cluster_count() const
{
  return cluster_count_;
}

uint32_t NMXHists::bin_width() const
{
  return pow(2, downshift_);
}

void NMXHists::clear() {
  memset(x_strips_hist, 0, sizeof(x_strips_hist));
  memset(y_strips_hist, 0, sizeof(y_strips_hist));
  memset(x_adc_hist, 0, sizeof(x_adc_hist));
  memset(y_adc_hist, 0, sizeof(y_adc_hist));
  memset(cluster_adc_hist, 0, sizeof(cluster_adc_hist));
  eventlet_count_ = 0;
  cluster_count_ = 0;
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
  eventlet_count_++;
}

void NMXHists::bin(const EventNMX& e)
{
  uint32_t sum = e.x.integral + e.y.integral;
  if (!sum)
    return;
  cluster_adc_hist[static_cast<uint16_t>(sum >> downshift_)]++;
  cluster_count_++;
}
