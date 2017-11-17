/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Trace.h>
#include <gdgem/nmx/HistSerializer.h>

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

HistSerializer::HistSerializer()
    : builder(2 * NMXHists::needed_buffer_size() + 256)
{}

HistSerializer::~HistSerializer() {}

size_t HistSerializer::serialize(const NMXHists& hists, char **buffer)
{
  builder.Clear();
  auto x_strip_off =
      builder.CreateUninitializedVector(hists.x_strips_hist.size(),
                                        hists.elem_size, &xtrackptr);
  auto y_strip_off =
      builder.CreateUninitializedVector(hists.y_strips_hist.size(),
                                        hists.elem_size, &ytrackptr);

  auto x_adc_off =
      builder.CreateUninitializedVector(hists.x_adc_hist.size(),
                                        hists.elem_size, &xadcptr);
  auto y_adc_off =
      builder.CreateUninitializedVector(hists.y_adc_hist.size(),
                                        hists.elem_size, &yadcptr);
  auto clus_adc_off =
      builder.CreateUninitializedVector(hists.cluster_adc_hist.size(),
                                        hists.elem_size, &clus_adc_ptr);

  memcpy(xtrackptr, &hists.x_strips_hist[0],
      hists.x_strips_hist.size() * hists.elem_size);
  memcpy(ytrackptr, &hists.y_strips_hist[0],
      hists.y_strips_hist.size() * hists.elem_size);
  memcpy(xadcptr, &hists.x_adc_hist[0],
      hists.x_adc_hist.size() * hists.elem_size);
  memcpy(yadcptr, &hists.y_adc_hist[0],
      hists.y_adc_hist.size() * hists.elem_size);
  memcpy(clus_adc_ptr, &hists.cluster_adc_hist[0],
      hists.cluster_adc_hist.size() * hists.elem_size);

  auto dataoff = CreateGEMHist(builder,
                               x_strip_off, y_strip_off,
                               x_adc_off, y_adc_off,
                               clus_adc_off,
                               hists.bin_width());

  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMHist, dataoff.Union());

  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  return builder.GetSize();
}
