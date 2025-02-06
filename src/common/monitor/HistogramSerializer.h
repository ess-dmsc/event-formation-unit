// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "mo01_nmx_generated.h"

#include <common/kafka/Producer.h>
#include <common/monitor/Histogram.h>

class HistogramSerializer {
public:
  /// \todo document
  HistogramSerializer(size_t buffer_half_size, const std::string &source_name);

  void set_callback(ProducerCallback cb);

  /// \todo document
  size_t produce(const Hists &hists);

private:
  ProducerCallback producer_callback;

  flatbuffers::FlatBufferBuilder builder;

  std::string SourceName;

  uint8_t *xtrackptr{nullptr};
  uint8_t *ytrackptr{nullptr};

  uint8_t *xadcptr{nullptr};
  uint8_t *yadcptr{nullptr};
  uint8_t *clus_adc_ptr{nullptr};
};
