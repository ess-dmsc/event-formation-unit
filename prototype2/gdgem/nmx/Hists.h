/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to histogram raw amd clustered VMM data for NMX
 */

#pragma once

#include <gdgem/nmx/EventNMX.h>
#include <math.h>
#include <vector>

class NMXHists
{
  public:
    using nmx_hist_type = uint32_t;
    static constexpr size_t elem_size { sizeof(nmx_hist_type) };

  public:
    std::vector<nmx_hist_type> x_strips_hist;
    std::vector<nmx_hist_type> y_strips_hist;
    std::vector<nmx_hist_type> x_adc_hist;
    std::vector<nmx_hist_type> y_adc_hist;
    std::vector<nmx_hist_type> cluster_adc_hist;

  public:
    NMXHists();

    void set_cluster_adc_downshift(uint32_t bits);

    /** @brief clears histograms */
    void clear();

    void bin(const Eventlet& e);
    void bin(const EventNMX& e);

    bool empty() const;
    size_t eventlet_count() const;
    size_t cluster_count() const;

    uint32_t bin_width() const;
    static size_t needed_buffer_size();

  private:
    uint32_t downshift_{0};
    size_t eventlet_count_{0};
    size_t cluster_count_{0};

    static size_t strip_hist_size();
    static size_t adc_hist_size();
};

