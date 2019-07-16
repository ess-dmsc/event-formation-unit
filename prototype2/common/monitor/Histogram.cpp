/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/monitor/Histogram.h>
#include <cmath>

size_t Hists::strip_hist_size() { return strip_max_val + 1; }

size_t Hists::adc_hist_size() { return adc_max_val + 1; }

size_t Hists::needed_buffer_size() {
  return elem_size *
      (strip_hist_size() * 2 + adc_hist_size() * 3 + 1 /*bin_width*/);
}

Hists::Hists(size_t strip_max, size_t adc_max)
    : strip_max_val(strip_max), adc_max_val(adc_max) {
  x_strips_hist.resize(strip_hist_size(), 0);
  y_strips_hist.resize(strip_hist_size(), 0);
  x_adc_hist.resize(adc_hist_size(), 0);
  y_adc_hist.resize(adc_hist_size(), 0);
  cluster_adc_hist.resize(adc_hist_size(), 0);
}

void Hists::set_cluster_adc_downshift(uint8_t bits) {
  if (bits > 32)
    bits = 32;
  downshift_ = bits;
}

bool Hists::isEmpty() const { return !(hit_count_ || cluster_count_); }

size_t Hists::hit_count() const { return hit_count_; }

size_t Hists::cluster_count() const { return cluster_count_; }

size_t Hists::bin_width() const { return static_cast<size_t>(pow(2, downshift_)); }

void Hists::clear() {
  std::fill(x_strips_hist.begin(), x_strips_hist.end(), 0);
  std::fill(y_strips_hist.begin(), y_strips_hist.end(), 0);
  std::fill(x_adc_hist.begin(), x_adc_hist.end(), 0);
  std::fill(y_adc_hist.begin(), y_adc_hist.end(), 0);
  std::fill(cluster_adc_hist.begin(), cluster_adc_hist.end(), 0);
  hit_count_ = 0;
  cluster_count_ = 0;
}

void Hists::bin_x(uint16_t xstrip, uint16_t xadc) {
  x_strips_hist[xstrip]++;
  x_adc_hist[xadc]++;
  hit_count_++;
}

void Hists::bin_y(uint16_t ystrip, uint16_t yadc) {
  y_strips_hist[ystrip]++;
  y_adc_hist[yadc]++;
  hit_count_++;
}

void Hists::bincluster(uint32_t sum) {
  if (!sum) {
    return;
  }
  cluster_adc_hist[sum >> downshift_]++;
  cluster_count_++;
}
