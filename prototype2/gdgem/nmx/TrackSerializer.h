/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
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

class TrackSerializer {
public:
  /** @todo document */
  TrackSerializer(size_t maxarraylength,
                  size_t minhits, double target_res);

  /** @todo document */
  ~TrackSerializer();

  /** @todo document */
  // int add_track(uint32_t plane, uint32_t strip, uint32_t time, uint32_t adc);
  int add_track(const Event &event);

  /** @todo document */
  int serialize(char **buffer);

private:
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
