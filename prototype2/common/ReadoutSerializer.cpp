/** Copyright (C) 2018 European Spallation Source ERIC */

#include <cinttypes>
#include <common/ReadoutSerializer.h>
#include <common/Trace.h>
#include <libs/include/gccintel.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define ELEMENTSIZE 10

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

ReadoutSerializer::ReadoutSerializer(size_t maxarraylength, Producer &prod)
    : maxlen(maxarraylength), builder(maxarraylength * ELEMENTSIZE + 256),
      producer(prod) {
      builder.Clear();
}

ReadoutSerializer::~ReadoutSerializer() {}

int ReadoutSerializer::produce() {
  auto planevec = builder.CreateVector(planes);
  auto timevec = builder.CreateVector(times);
  auto channelvec = builder.CreateVector(channels);
  auto adcvec = builder.CreateVector(adcs);

  auto dataoff = CreateMONHit(builder, planevec, timevec, channelvec, adcvec);
  auto msg = CreateMonitorMessage(builder, 0, DataField::MONHit, dataoff.Union());
  builder.Finish(msg);

  char * buffer = (char *)builder.GetBufferPointer();
  auto buffersize = builder.GetSize();
  producer.produce(buffer, buffersize);

  planes.clear();
  channels.clear();
  times.clear();
  adcs.clear();
  entries = 0;
  builder.Clear();

  return buffersize;
}

int ReadoutSerializer::addEntry(uint16_t plane, uint16_t channel, uint32_t time, uint16_t adc) {
  int ret = 0;

  planes.push_back(plane);
  channels.push_back(channel);
  times.push_back(time);
  adcs.push_back(adc);
  entries++;

  if (entries == maxlen) {
    ret = produce(); // entries will be set to 0
  }
  return ret;
}
