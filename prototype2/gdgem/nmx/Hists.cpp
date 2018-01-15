/** Copyright (C) 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hists.h>
#include <string.h>

// static

size_t NMXHists::strip_hist_size() { return Eventlet::strip_max_val + 1; }

size_t NMXHists::adc_hist_size() { return Eventlet::adc_max_val + 1; }

size_t NMXHists::needed_buffer_size() {
  return elem_size *
         (strip_hist_size() * 2 + adc_hist_size() * 3 + 1 /*bin_width*/);
}

NMXHists::NMXHists() {
  x_strips_hist.resize(strip_hist_size(), 0);
  y_strips_hist.resize(strip_hist_size(), 0);
  x_adc_hist.resize(adc_hist_size(), 0);
  y_adc_hist.resize(adc_hist_size(), 0);
  cluster_adc_hist.resize(adc_hist_size(), 0);
}

void NMXHists::set_cluster_adc_downshift(uint32_t bits) {
  if (bits > 32)
    bits = 32;
  downshift_ = bits;
}

bool NMXHists::isEmpty() const { return !(eventlet_count_ || cluster_count_); }

size_t NMXHists::eventlet_count() const { return eventlet_count_; }

size_t NMXHists::cluster_count() const { return cluster_count_; }

uint32_t NMXHists::bin_width() const { return pow(2, downshift_); }

void NMXHists::clear() {
  std::fill(x_strips_hist.begin(), x_strips_hist.end(), 0);
  std::fill(y_strips_hist.begin(), y_strips_hist.end(), 0);
  std::fill(x_adc_hist.begin(), x_adc_hist.end(), 0);
  std::fill(y_adc_hist.begin(), y_adc_hist.end(), 0);
  std::fill(cluster_adc_hist.begin(), cluster_adc_hist.end(), 0);
  eventlet_count_ = 0;
  cluster_count_ = 0;
}

void NMXHists::bin(const Eventlet &e) {
  if (e.plane_id == 0) {
    x_strips_hist[e.strip]++;
    x_adc_hist[e.adc]++;
  } else if (e.plane_id == 1) {
    y_strips_hist[e.strip]++;
    y_adc_hist[e.adc]++;
  } else
    return;
  eventlet_count_++;
}

// To be used for multigrid also - consider refactoring and moving elsewhere
void NMXHists::binstrips(uint16_t xstrip, uint16_t xadc, uint16_t ystrip, uint16_t yadc) {
    x_strips_hist[xstrip]++;
    x_adc_hist[xadc]++;
    y_strips_hist[ystrip]++;
    y_adc_hist[yadc]++;
    eventlet_count_++;
}

void NMXHists::bin(const EventNMX &e) {
  uint32_t sum = e.x.integral + e.y.integral;
  if (!sum)
    return;
  cluster_adc_hist[static_cast<uint16_t>(sum >> downshift_)]++;
  cluster_count_++;
}
