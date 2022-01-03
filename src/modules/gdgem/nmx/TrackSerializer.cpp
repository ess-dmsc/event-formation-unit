/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/TrackSerializer.h>

#include <common/debug/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/debug/Log.h>
//#undef TRC_MASK
//#define TRC_MASK 0

#define EV_ELEMSIZE sizeof(uint16_t)
#define EV_SIZE (3 * EV_ELEMSIZE)
#define POS_SIZE sizeof(double)
#define TIME_OFFSET_SIZE sizeof(uint64_t)

#define BUF_STATIC_SIZE (2 * POS_SIZE + TIME_OFFSET_SIZE)


static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

namespace Gem {

TrackSerializer::TrackSerializer(size_t maxarraylength, std::string source_name)
    : maxlen(maxarraylength)
    , builder(maxlen * EV_SIZE * 2 + BUF_STATIC_SIZE + 256)
    , SourceName (source_name) {
  builder.Clear();
}

void TrackSerializer::set_callback(ProducerCallback cb) {
  producer_callback = cb;
}

bool TrackSerializer::add_track(const Event &event, double utpc_x, double utpc_y) {

  if ((event.ClusterA.hit_count() > maxlen) || (event.ClusterB.hit_count() > maxlen)) {
    return false;
  }

  time_offset = event.time_start();

  for (auto &evx : event.ClusterA.hits) {
    xtrack.push_back(
        Createpos(builder,
                  static_cast<uint16_t>((evx.time - time_offset)),
                  evx.coordinate, evx.weight));
  }

  for (auto &evy : event.ClusterB.hits) {
    ytrack.push_back(
        Createpos(builder,
                  static_cast<uint16_t>((evy.time - time_offset)),
                  evy.coordinate, evy.weight));
  }

  xpos = utpc_x;
  ypos = utpc_y;

  if (producer_callback) {
    auto buffer = serialize();
    XTRACE(PROCESS, INF, "Producing track as buffer size: %d", buffer.size_bytes());
#pragma message("Producer::produce() in TrackSerializer should be provided with a proper timestamp.")
    producer_callback(buffer, time(nullptr) * 1000);
    return (0 != buffer.size_bytes());
  }

  return true;
}

nonstd::span<uint8_t> TrackSerializer::serialize() {
  if ((xtrack.size() == 0) && (ytrack.size() == 0)) {
    return {};
  }

  auto SourceNameOffset = builder.CreateString(SourceName);

  auto xtrackvec = builder.CreateVector(xtrack);
  auto ytrackvec = builder.CreateVector(ytrack);
  auto dataoff =
      CreateGEMTrack(builder, time_offset, xtrackvec, ytrackvec, xpos, ypos);
  auto msg =
      CreateMonitorMessage(builder, SourceNameOffset, DataField::GEMTrack, dataoff.Union());
  FinishMonitorMessageBuffer(builder, msg);
  xtrack.clear();
  ytrack.clear();
  nonstd::span<uint8_t> ret(builder.GetBufferPointer(), builder.GetSize());

  builder.Clear();

  return ret;
}

}
