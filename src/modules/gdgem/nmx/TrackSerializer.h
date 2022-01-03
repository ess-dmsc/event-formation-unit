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

#include <common/reduction/Event.h>
#include <common/kafka/Producer.h>

namespace Gem {

class TrackSerializer {
public:
  /// \todo document
  TrackSerializer(size_t maxarraylength, std::string source_name);

  void set_callback(ProducerCallback cb);

  /// \todo document
  /// \returns success
  bool add_track(const Event &event, double utpc_x, double utpc_y);

  /// \todo document
  nonstd::span<uint8_t> serialize();

private:
  size_t maxlen{0};
  flatbuffers::FlatBufferBuilder builder;
 
  std::string SourceName;

  ProducerCallback producer_callback;

  uint64_t time_offset{0};
  std::vector<flatbuffers::Offset<pos>> xtrack;
  std::vector<flatbuffers::Offset<pos>> ytrack;
  double xpos{-1};
  double ypos{-1};
};

}
