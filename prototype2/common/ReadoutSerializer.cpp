/** Copyright (C) 2018 European Spallation Source ERIC */

#include <cinttypes>
#include <common/ReadoutSerializer.h>
#include <common/Trace.h>
#include <common/gccintel.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

ReadoutSerializer::ReadoutSerializer(size_t maxarraylength, std::string source_name)
    : maxlen(maxarraylength), SourceName(source_name) {
  builder.Clear();
}

void ReadoutSerializer::set_callback(ProducerCallback cb) {
  producer_callback = cb;
}

// \todo labels for planes
// \todo offset time

size_t ReadoutSerializer::produce() {
  if (entries == 0) {
    return 0;
  }

  auto SourceNameOffset = builder.CreateString(SourceName);

  auto planevec = builder.CreateVector(planes);
  auto timevec = builder.CreateVector(times);
  auto channelvec = builder.CreateVector(channels);
  auto adcvec = builder.CreateVector(adcs);

  auto dataoff = CreateMONHit(builder, planevec, timevec, channelvec, adcvec);
  auto msg = CreateMonitorMessage(builder, SourceNameOffset,
                                  DataField::MONHit, dataoff.Union());
  FinishMonitorMessageBuffer(builder, msg);

  Buffer<uint8_t> buffer(builder.GetBufferPointer(), builder.GetSize());
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
