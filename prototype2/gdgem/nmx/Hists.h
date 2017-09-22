/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to histogram NMX strip data in x and y dimensions
 */

#pragma once

#include <cinttypes>
#include "Eventlet.h"

#define NMX_STRIP_HIST_SIZE NMX_STRIP_MAX_VAL + 1
#define NMX_ADC_HIST_SIZE NMX_ADC_MAX_VAL + 1

#define NMX_HIST_TYPE uint32_t
#define NMX_HIST_ELEM_SIZE sizeof(NMX_HIST_TYPE)

struct NMXHists
{
    NMXHists();

    /** @brief clears histograms for x and y strips */
    void clear();

    void bin(const Eventlet& e);

    NMX_HIST_TYPE x_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE y_strips_hist[NMX_STRIP_HIST_SIZE];
    NMX_HIST_TYPE x_adc_hist[NMX_ADC_HIST_SIZE];
    NMX_HIST_TYPE y_adc_hist[NMX_ADC_HIST_SIZE];

    NMX_HIST_TYPE cluster_adc_hist[NMX_ADC_HIST_SIZE];
    uint32_t downshift_{10};

    uint32_t xyhist_elems{0};

};
