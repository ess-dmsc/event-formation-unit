/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "mo01_nmx_generated.h"

#include <common/Producer.h>
#include <common/Hists.h>
#include <libs/include/gccintel.h>

class HistSerializer {
public:
  /** \todo document */
  HistSerializer(size_t buffer_half_size);

  /** \todo document */
  ~HistSerializer();

  /** \todo document */
  size_t serialize(const NMXHists &hists, char **buffer);

private:
  flatbuffers::FlatBufferBuilder builder;
  uint8_t *xtrackptr{nullptr};
  uint8_t *ytrackptr{nullptr};

  uint8_t *xadcptr{nullptr};
  uint8_t *yadcptr{nullptr};
  uint8_t *clus_adc_ptr{nullptr};
};
