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

#include <gdgem/nmx/Event.h>
#include <common/Producer.h>

class TrackSerializer {
public:
  /// \todo document
  TrackSerializer(size_t maxarraylength,
                  size_t minhits, double target_res);

  void set_callback(ProducerCallback cb);

  /// \todo document
  /// \returns success
  bool add_track(const Event &event);

  /// \todo document
  Buffer<uint8_t> serialize();

private:
  ProducerCallback producer_callback;

  flatbuffers::FlatBufferBuilder builder;
  size_t maxlen{0};
  size_t minhits_{0};
  double target_resolution_ {1};

  double time_offset{0};
  std::vector<flatbuffers::Offset<pos>> xtrack;
  std::vector<flatbuffers::Offset<pos>> ytrack;
  double xpos{-1};
  double ypos{-1};
};
