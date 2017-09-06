/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Trace.h>
#include <gdgem/nmx/TrackSerializer.h>

#define ELEMSIZE 4

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

TrackSerializer::TrackSerializer(size_t maxarraylength)
    : builder(maxarraylength * ELEMSIZE * 3 * 2 + 256), maxlen(maxarraylength) {
  builder.Clear();
}

TrackSerializer::~TrackSerializer() {}

int TrackSerializer::add_track(const EventNMX &event, size_t minhits) {
  if ((event.x.entries.size() < minhits) ||
      (event.y.entries.size() < minhits)) {
    return 1;
  }

  if ((event.x.entries.size() > maxlen) || (event.y.entries.size() > maxlen)) {
    return 1;
  }
  auto timeoff = event.time_start();

  for (auto &evx : event.x.entries) {
    xpos.push_back(Createpos(builder, evx.strip, evx.time - timeoff, evx.adc));
  }

  for (auto &evy : event.y.entries) {
    ypos.push_back(Createpos(builder, evy.strip, evy.time - timeoff, evy.adc));
  }
  return 0;
}

int TrackSerializer::serialize(char **buffer) {
  if ((xpos.size() == 0) || (ypos.size() == 0)) {
    *buffer = 0;
    return 0;
  }

  auto xposvec = builder.CreateVector(xpos);
  auto yposvec = builder.CreateVector(ypos);
  auto dataoff = CreateGEMTrack(builder, xposvec, yposvec);
  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMTrack, dataoff.Union());
  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  xpos.clear();
  ypos.clear();
  auto buffersize = builder.GetSize();
  builder.Clear();
  return buffersize;
}
