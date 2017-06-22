/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/HistSerializer.h>
#include <common/Trace.h>
#include <cinttypes>

#define ELEMSIZE 4

static_assert(FLATBUFFERS_LITTLEENDIAN, "Flatbuffers only tested on little endian systems");

HistSerializer::HistSerializer(size_t maxarraylength)
    : builder(maxarraylength * ELEMSIZE * 2 + 256)
    , maxlen(maxarraylength) { }

HistSerializer::~HistSerializer() {}

int HistSerializer::serialize(uint32_t * xhist, uint32_t * yhist, size_t entries, char **buffer) {
  if (entries > maxlen) {
    *buffer = 0;
    return 0;
  }

  builder.Clear();
  auto xoff = builder.CreateUninitializedVector(entries, ELEMSIZE, &xarrptr);
  auto yoff = builder.CreateUninitializedVector(entries, ELEMSIZE, &yarrptr);
  memcpy(xarrptr, xhist, entries * ELEMSIZE);
  memcpy(yarrptr, yhist, entries * ELEMSIZE);
  auto dataoff = CreateGEMHist(builder, xoff, yoff);

  auto msg = CreateMonitorMessage(builder, 0, DataField_GEMHist, dataoff.Union());

  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  return builder.GetSize();
}
