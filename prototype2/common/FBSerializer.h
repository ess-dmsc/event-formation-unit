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

class FBSerializer {
public:
  FBSerializer(size_t maxarraylength);

  ~FBSerializer();

  int serialize(uint64_t time, uint64_t seqno, char *timearr, char *pixarr,
                size_t entries, unsigned char **buffer, size_t *reslen);

private:
  flatbuffers::FlatBufferBuilder builder;
  size_t maxlen{0};
  uint8_t *timeptr{nullptr};
  uint8_t *pixelptr{nullptr};
};
