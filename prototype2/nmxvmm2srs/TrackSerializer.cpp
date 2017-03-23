/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/TrackSerializer.h>
#include <common/Trace.h>
#include <cinttypes>

#define ELEMSIZE 4

static_assert(FLATBUFFERS_LITTLEENDIAN, "Flatbuffers only tested on little endian systems");

TrackSerializer::TrackSerializer(size_t maxarraylength)
    : builder(maxarraylength * ELEMSIZE * 3 * 2 + 256)
    , maxlen(maxarraylength) {
      builder.Clear();
    }

TrackSerializer::~TrackSerializer() { }

int TrackSerializer::add_track(uint32_t plane, uint32_t strip, uint32_t time, uint32_t adc) {
  if (plane == 0) {
    if (xentries == maxlen) {
      return -1;
    }
    if (plane == 0) {
      xpos.push_back(Createpos(builder, strip, time, adc));
      xentries++;
    }

    return xentries;
  } else if (plane == 1) {
    if (yentries == maxlen) {
      return -1;
    }
    ypos.push_back(Createpos(builder, strip, time, adc));
    yentries++;
    return yentries;
  }
  return -1;
}

int TrackSerializer::serialize(char **buffer) {
  if (xentries == 0 || yentries == 0) {
    *buffer = 0;
    return 0;
  }

  auto xposvec = builder.CreateVector(xpos);
  auto yposvec = builder.CreateVector(ypos);
  auto dataoff = CreateGEMTrack(builder, xposvec, yposvec);
  auto msg = CreateMonitorMessage(builder, 0, DataField_GEMTrack, dataoff.Union());
  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  xpos.clear();
  ypos.clear();
  xentries = 0;
  yentries = 0;
  auto buffersize = builder.GetSize();
  builder.Clear();
  return buffersize;
}
