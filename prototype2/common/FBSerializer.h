/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief flatbuffer serialization
 */

#pragma once

#ifdef FLATBUFFERS
#include <../streaming-data-types/build/schemas/ev42_events_generated.h>
#else
#pragma message("FLATBUFFERS not defined, using old schemas")
#include <common/ev42_events_generated.h>
#endif
#include <common/Producer.h>

class FBSerializer {
public:

  /** @todo document */
  FBSerializer(size_t maxarraylength, Producer &prod);

  /** @todo document */
  ~FBSerializer();

  /** @todo document */
  int serialize(uint64_t time, uint64_t seqno, size_t entries, char **buffer);

  /** @todo document */
  int addevent(uint32_t time, uint32_t pixel);

  /** @todo document */
  int produce();

private:
  flatbuffers::FlatBufferBuilder builder;
  size_t maxlen{0};
  uint8_t *timeptr{nullptr};
  uint8_t *pixelptr{nullptr};
  Producer & producer;

  size_t events{0};
  size_t seqno{1};
};
