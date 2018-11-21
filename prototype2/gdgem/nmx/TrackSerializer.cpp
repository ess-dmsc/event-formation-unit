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

namespace Gem {

TrackSerializer::TrackSerializer(size_t maxarraylength, size_t minhits, double target_res)
    : builder(maxarraylength * EV_SIZE * 2 + BUF_STATIC_SIZE + 256),
      maxlen(maxarraylength), minhits_(minhits), target_resolution_(target_res) {
  builder.Clear();
}

void TrackSerializer::set_callback(ProducerCallback cb) {
  producer_callback = cb;
}

bool TrackSerializer::add_track(const Event &event) {
  if ((event.x.hits.size() < minhits_) ||
      (event.y.hits.size() < minhits_)) {
    return false;
  }

  if ((event.x.hits.size() > maxlen) || (event.y.hits.size() > maxlen)) {
    return false;
  }

  time_offset = event.time_start();

  for (auto &evx : event.x.hits) {
    xtrack.push_back(
        Createpos(builder,
                  static_cast<uint16_t>((evx.time - time_offset) * target_resolution_),
                  evx.strip, evx.adc));
  }

  for (auto &evy : event.y.hits) {
    ytrack.push_back(
        Createpos(builder,
                  static_cast<uint16_t>((evy.time - time_offset) * target_resolution_),
                  evy.strip, evy.adc));
  }

  xpos = event.x.utpc_center;
  ypos = event.y.utpc_center;

  if (producer_callback) {
    auto buffer = serialize();
    producer_callback(buffer);
    return (0 != buffer.size);
  }

  return true;
}

Buffer<uint8_t> TrackSerializer::serialize() {
  if ((xtrack.size() == 0) || (ytrack.size() == 0)) {
    return Buffer<uint8_t>();
  }

  auto xtrackvec = builder.CreateVector(xtrack);
  auto ytrackvec = builder.CreateVector(ytrack);
  auto dataoff =
      CreateGEMTrack(builder, time_offset, xtrackvec, ytrackvec, xpos, ypos);
  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMTrack, dataoff.Union());
  builder.Finish(msg);
  xtrack.clear();
  ytrack.clear();
  Buffer<uint8_t> ret(builder.GetBufferPointer(), builder.GetSize());

  builder.Clear();

  return ret;
}

}
