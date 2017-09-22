/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to histogram raw amd clustered VMM data for NMX
 */

#pragma once

#include "EventNMX.h"
#include <math.h>

#define NMX_STRIP_HIST_SIZE ( NMX_STRIP_MAX_VAL + 1 )
#define NMX_ADC_HIST_SIZE ( NMX_ADC_MAX_VAL + 1 )

#define NMX_HIST_TYPE uint32_t
#define NMX_HIST_ELEM_SIZE sizeof(NMX_HIST_TYPE)

struct NMXHists
{
    NMXHists();

    /** @brief clears histograms for x and y strips */
    void clear();

    void bin(const Eventlet& e);
    void bin(const EventNMX& e);

    NMX_HIST_TYPE x_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE y_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE x_adc_hist[NMX_ADC_HIST_SIZE];
    NMX_HIST_TYPE y_adc_hist[NMX_ADC_HIST_SIZE];

    NMX_HIST_TYPE cluster_adc_hist[NMX_ADC_HIST_SIZE];
    uint32_t downshift_{6};

    uint32_t bin_width() const
    {
      return pow(2, downshift_);
    }

    uint32_t xyhist_elems{0};

    inline static size_t needed_buffer_size()
    {
      return
          NMX_HIST_ELEM_SIZE * NMX_STRIP_HIST_SIZE * 2 +
          NMX_HIST_ELEM_SIZE * NMX_ADC_HIST_SIZE * 3 +
          sizeof(uint32_t); //bin_width
    }

};
