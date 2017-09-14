/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to histogram NMX strip data in x and y dimensions
 */

#pragma once

#include <cinttypes>

struct NMXHists
{
    NMXHists();

    /** @brief clears histograms for x and y strips */
    void clear();
    void bin_one(uint16_t plane_id, uint16_t strip);

    uint32_t xyhist[2][1500];
    uint32_t xyhist_elems{0};
};
