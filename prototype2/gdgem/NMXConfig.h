/** Copyright (C) 2017 European Spallation Source ERIC */

#pragma once

#include <string>
#include <cinttypes>
#include <gdgem/nmx/Geometry.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>

struct NMXConfig
{
    NMXConfig() {}
    NMXConfig(std::string jsonfile);

    // geometry
    size_t geometry_x {256};
    size_t geometry_y {256};

    std::string builder_type {"SRS"};

    // SRS only
    SRSTime time_config;
    SRSMappings srs_mappings;

    //runtime
    uint64_t cluster_min_timespan {30};
    bool analyze_weighted {true};
    int16_t analyze_max_timebins {3};
    int16_t analyze_max_timedif {7};
    size_t track_sample_minhits {6};

    std::string debug() const;
};
