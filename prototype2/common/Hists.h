/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to histogram raw amd clustered VMM data for NMX
///
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>
#include <cinttypes>
#include <cstddef>

class Hists {
public:
  using nmx_hist_type = uint32_t;
  static constexpr size_t elem_size{sizeof(nmx_hist_type)};

public:
  std::vector<nmx_hist_type> x_strips_hist;
  std::vector<nmx_hist_type> y_strips_hist;
  std::vector<nmx_hist_type> x_adc_hist;
  std::vector<nmx_hist_type> y_adc_hist;
  std::vector<nmx_hist_type> cluster_adc_hist;

public:
  Hists(size_t strip_max, size_t adc_max);

  void set_cluster_adc_downshift(uint32_t bits);

  /// \brief clears histograms
  void clear();

  void binstrips(uint16_t xstrip, uint16_t xadc, uint16_t ystrip, uint16_t yadc);
  void bincluster(uint32_t sum);

  bool isEmpty() const;
  size_t hit_count() const;
  size_t cluster_count() const;

  uint32_t bin_width() const;

  size_t needed_buffer_size();
  size_t strip_hist_size();
  size_t adc_hist_size();

private:
  uint32_t downshift_{0};
  size_t hit_count_{0};
  size_t cluster_count_{0};

  size_t strip_max_val;
  size_t adc_max_val;
};
