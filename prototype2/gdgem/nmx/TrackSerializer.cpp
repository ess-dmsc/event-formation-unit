/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Trace.h>
#include <gdgem/nmx/TrackSerializer.h>

#define EV_ELEMSIZE sizeof(uint16_t)
#define EV_SIZE (3 * EV_ELEMSIZE)
#define POS_SIZE sizeof(double)
#define TIME_OFFSET_SIZE sizeof(uint64_t)

#define BUF_STATIC_SIZE (2 * POS_SIZE + TIME_OFFSET_SIZE)

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

TrackSerializer::TrackSerializer(size_t maxarraylength, size_t minhits)
    : builder(maxarraylength * EV_SIZE * 2 + BUF_STATIC_SIZE + 256),
      maxlen(maxarraylength), minhits_(minhits) {
  builder.Clear();
}

TrackSerializer::~TrackSerializer() {}

int TrackSerializer::add_track(const EventNMX &event) {
  if ((event.x.entries.size() < minhits_) ||
      (event.y.entries.size() < minhits_)) {
    return 1;
  }

  if ((event.x.entries.size() > maxlen) || (event.y.entries.size() > maxlen)) {
    return 1;
  }

  time_offset = event.time_start();

  for (auto &evx : event.x.entries) {
    xtrack.push_back(
        Createpos(builder, evx.time - time_offset, evx.strip, evx.adc));
  }

  for (auto &evy : event.y.entries) {
    ytrack.push_back(
        Createpos(builder, evy.time - time_offset, evy.strip, evy.adc));
  }

  xpos = event.x.center;
  ypos = event.y.center;

  return 0;
}

int TrackSerializer::serialize(char **buffer) {
  if ((xtrack.size() == 0) || (ytrack.size() == 0)) {
    *buffer = 0;
    return 0;
  }

  auto xtrackvec = builder.CreateVector(xtrack);
  auto ytrackvec = builder.CreateVector(ytrack);
  auto dataoff =
      CreateGEMTrack(builder, time_offset, xtrackvec, ytrackvec, xpos, ypos);
  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMTrack, dataoff.Union());
  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  xtrack.clear();
  ytrack.clear();
  auto buffersize = builder.GetSize();
  builder.Clear();
  return buffersize;
}
