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
//  return 0;
  builder.Clear();
  auto x_strip_off = builder.CreateUninitializedVector(NMX_STRIP_HIST_SIZE, NMX_HIST_ELEM_SIZE, &xtrackptr);
  auto y_strip_off = builder.CreateUninitializedVector(NMX_STRIP_HIST_SIZE, NMX_HIST_ELEM_SIZE, &ytrackptr);

  auto x_adc_off = builder.CreateUninitializedVector(NMX_ADC_HIST_SIZE, NMX_HIST_ELEM_SIZE, &xadcptr);
  auto y_adc_off = builder.CreateUninitializedVector(NMX_ADC_HIST_SIZE, NMX_HIST_ELEM_SIZE, &yadcptr);
  auto clus_adc_off = builder.CreateUninitializedVector(NMX_ADC_HIST_SIZE, NMX_HIST_ELEM_SIZE, &clus_adc_ptr);

  memcpy(xtrackptr, &hists.x_strips_hist[0], NMX_STRIP_HIST_SIZE * NMX_HIST_ELEM_SIZE);
  memcpy(ytrackptr, &hists.y_strips_hist[0], NMX_STRIP_HIST_SIZE * NMX_HIST_ELEM_SIZE);
  memcpy(xadcptr, &hists.x_adc_hist[0], NMX_ADC_HIST_SIZE * NMX_HIST_ELEM_SIZE);
  memcpy(yadcptr, &hists.y_adc_hist[0], NMX_ADC_HIST_SIZE * NMX_HIST_ELEM_SIZE);
  memcpy(clus_adc_ptr, &hists.cluster_adc_hist[0], NMX_ADC_HIST_SIZE * NMX_HIST_ELEM_SIZE);

  auto dataoff = CreateGEMHist(builder,
                               x_strip_off, y_strip_off
                               , x_adc_off, y_adc_off
                               , clus_adc_off
                               , hists.bin_width());

  auto msg =
      CreateMonitorMessage(builder, 0, DataField::GEMHist, dataoff.Union());

  builder.Finish(msg);
  *buffer = (char *)builder.GetBufferPointer();
  return builder.GetSize();
}
