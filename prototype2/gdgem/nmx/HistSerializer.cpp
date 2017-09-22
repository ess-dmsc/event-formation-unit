/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Trace.h>
#include <gdgem/nmx/HistSerializer.h>

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

HistSerializer::HistSerializer(size_t maxarraylength)
    : builder(maxarraylength * NMX_HIST_ELEM_SIZE * 2 + 256)
    , maxlen(maxarraylength) {}

HistSerializer::~HistSerializer() {}

size_t HistSerializer::serialize(const NMXHists& hists, char **buffer)
{
  return serialize(&hists.x_strips_hist[0], &hists.y_strips_hist[0],
      NMX_STRIP_HIST_SIZE, buffer);
}

size_t HistSerializer::serialize(const NMX_HIST_TYPE *x_strips_hist,
                                 const NMX_HIST_TYPE *y_strips_hist,
                                 size_t entries, char **buffer) {
  if (entries > maxlen) {
    *buffer = 0;
    return 0;
  }

  builder.Clear();
  auto xoff = builder.CreateUninitializedVector(entries, NMX_HIST_ELEM_SIZE, &xarrptr);
  auto yoff = builder.CreateUninitializedVector(entries, NMX_HIST_ELEM_SIZE, &yarrptr);
  memcpy(xarrptr, x_strips_hist, entries * NMX_HIST_ELEM_SIZE);
  memcpy(yarrptr, y_strips_hist, entries * NMX_HIST_ELEM_SIZE);
  auto dataoff = CreateGEMHist(builder, xoff, yoff);

  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMHist, dataoff.Union());

  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  return builder.GetSize();
}
