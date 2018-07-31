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

#include <common/Hists.h>
#include <common/Buffer.h>
#include <functional>

class HistSerializer {
public:
  /** \todo document */
  HistSerializer(size_t buffer_half_size);

  void set_callback(std::function<void(Buffer)> cb);

  /** \todo document */
  size_t produce(const Hists &hists);

private:
  std::function<void(Buffer)> producer_callback;

  flatbuffers::FlatBufferBuilder builder;

  uint8_t *xtrackptr{nullptr};
  uint8_t *ytrackptr{nullptr};

  uint8_t *xadcptr{nullptr};
  uint8_t *yadcptr{nullptr};
  uint8_t *clus_adc_ptr{nullptr};
};
