/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "mo01_nmx_generated.h"

#include <common/clustering/Event.h>
#include <common/Producer.h>

namespace Gem {

class TrackSerializer {
public:
  /// \todo document
  TrackSerializer(size_t maxarraylength, double target_res, std::string source_name);

  void set_callback(ProducerCallback cb);

  /// \todo document
  /// \returns success
  bool add_track(const Event &event, double utpc_x, double utpc_y);

  /// \todo document
  Buffer<uint8_t> serialize();

private:
  size_t maxlen{0};
  flatbuffers::FlatBufferBuilder builder;
  double target_resolution_{1};

  std::string SourceName;

  ProducerCallback producer_callback;

  uint64_t time_offset{0};
  std::vector<flatbuffers::Offset<pos>> xtrack;
  std::vector<flatbuffers::Offset<pos>> ytrack;
  double xpos{-1};
  double ypos{-1};
};

}
