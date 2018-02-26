/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief flatbuffer serialization
 */

#pragma once

#include "mo01_nmx_generated.h"

#include <common/Producer.h>
#include <gdgem/nmx/Hists.h>
#include <libs/include/gccintel.h>

class HistSerializer {
public:
  /** @todo document */
  HistSerializer();

  /** @todo document */
  ~HistSerializer();

  /** @todo document */
  size_t serialize(const NMXHists &hists, char **buffer);

private:
  flatbuffers::FlatBufferBuilder builder;
  uint8_t *xtrackptr{nullptr};
  uint8_t *ytrackptr{nullptr};

  uint8_t *xadcptr{nullptr};
  uint8_t *yadcptr{nullptr};
  uint8_t *clus_adc_ptr{nullptr};
};
