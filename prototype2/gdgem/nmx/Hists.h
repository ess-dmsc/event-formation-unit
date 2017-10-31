/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to histogram raw amd clustered VMM data for NMX
 */

#pragma once

#include <gdgem/nmx/EventNMX.h>
#include <math.h>

#define NMX_STRIP_HIST_SIZE ( NMX_STRIP_MAX_VAL + 1 )
#define NMX_ADC_HIST_SIZE ( NMX_ADC_MAX_VAL + 1 )

#define NMX_HIST_TYPE uint32_t
#define NMX_HIST_ELEM_SIZE sizeof(NMX_HIST_TYPE)

class NMXHists
{
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

  public:
    NMX_HIST_TYPE x_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE y_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE x_adc_hist[NMX_ADC_HIST_SIZE];
    NMX_HIST_TYPE y_adc_hist[NMX_ADC_HIST_SIZE];
    NMX_HIST_TYPE cluster_adc_hist[NMX_ADC_HIST_SIZE];

  private:
    uint32_t downshift_{0};
    size_t eventlet_count_{0};
    size_t cluster_count_{0};
};
