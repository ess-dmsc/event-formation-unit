/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Bastract class for creating NMX eventlets
 */

#pragma once

#include <gdgem/nmx/Clusterer.h>
#include <gdgem/nmx/Hists.h>

class AbstractBuilder {
  public:

    struct ResultStats
    {
        ResultStats() {}
        ResultStats(uint32_t ev, uint32_t err)
          : valid_eventlets(ev)
          , error_bytes(err)
        {}

        uint32_t valid_eventlets {0};
        uint32_t error_bytes {0};
    };

    AbstractBuilder() {}
    virtual ~AbstractBuilder() {}

    /** @todo Martin document */
    virtual ResultStats process_buffer(char *buf, size_t size,
                                       Clusterer &clusterer,
                                       NMXHists &hists) = 0;

};
