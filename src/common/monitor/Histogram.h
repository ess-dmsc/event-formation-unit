// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to histogram raw amd clustered VMM data for NMX
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cstddef>
#include <vector>

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

  void set_cluster_adc_downshift(uint8_t bits);

  /// \brief clears histograms
  void clear();

  /// \brief assume range is good
  void bin_x(uint16_t xstrip, uint16_t xadc);

  /// \brief assume range is good
  void bin_y(uint16_t xstrip, uint16_t xadc);

  /// \brief assume range is good
  void bincluster(uint32_t sum);

  bool isEmpty() const;
  size_t hitCount() const;
  size_t cluster_count() const;

  size_t bin_width() const;

  size_t needed_buffer_size();
  size_t strip_hist_size();
  size_t adc_hist_size();

private:
  uint8_t downshift_{0};
  size_t hit_count_{0};
  size_t cluster_count_{0};

  size_t strip_max_val;
  size_t adc_max_val;
};
