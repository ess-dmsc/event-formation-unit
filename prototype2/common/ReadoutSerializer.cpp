/** Copyright (C) 2018 European Spallation Source ERIC */

#include <cinttypes>
#include <common/ReadoutSerializer.h>
#include <common/Trace.h>
#include <libs/include/gccintel.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

ReadoutSerializer::ReadoutSerializer(size_t maxarraylength)
    : maxlen(maxarraylength) {
      builder.Clear();
}

void ReadoutSerializer::set_callback(std::function<void(Buffer)> cb) {
  producer_callback = cb;
}

size_t ReadoutSerializer::produce() {
  if (entries == 0) {
    return 0;
  }
  auto planevec = builder.CreateVector(planes);
  auto timevec = builder.CreateVector(times);
  auto channelvec = builder.CreateVector(channels);
  auto adcvec = builder.CreateVector(adcs);

  auto dataoff = CreateMONHit(builder, planevec, timevec, channelvec, adcvec);
  auto msg = CreateMonitorMessage(builder, 0, DataField::MONHit, dataoff.Union());
  builder.Finish(msg);

  Buffer buffer;
  buffer.buffer = (char *)builder.GetBufferPointer();
  buffer.size = builder.GetSize();
  if (producer_callback) {
    producer_callback(buffer);
  }

  planes.clear();
  channels.clear();
  times.clear();
  adcs.clear();
  entries = 0;
  builder.Clear();

  return buffer.size;
}

size_t ReadoutSerializer::addEntry(uint16_t plane, uint16_t channel, uint32_t time, uint16_t adc) {
  planes.push_back(plane);
  channels.push_back(channel);
  times.push_back(time);
  adcs.push_back(adc);
  entries++;

  if (entries == maxlen) {
    return produce(); // entries will be set to 0
  }
  return 0;
}
