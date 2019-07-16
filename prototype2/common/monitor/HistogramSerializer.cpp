/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/monitor/HistogramSerializer.h>
#include <common/gccintel.h>

#include <common/Trace.h>

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

HistogramSerializer::HistogramSerializer(size_t buffer_half_size, std::string source_name)
    : builder(2 * buffer_half_size + 256)
    , SourceName (source_name) {}


void HistogramSerializer::set_callback(ProducerCallback cb) {
  producer_callback = cb;
}

size_t HistogramSerializer::produce(const Hists &hists) {
  builder.Clear();
  auto x_strip_off = builder.CreateUninitializedVector(
      hists.x_strips_hist.size(), hists.elem_size, &xtrackptr);
  auto y_strip_off = builder.CreateUninitializedVector(
      hists.y_strips_hist.size(), hists.elem_size, &ytrackptr);

  auto x_adc_off = builder.CreateUninitializedVector(hists.x_adc_hist.size(),
                                                     hists.elem_size, &xadcptr);
  auto y_adc_off = builder.CreateUninitializedVector(hists.y_adc_hist.size(),
                                                     hists.elem_size, &yadcptr);
  auto clus_adc_off = builder.CreateUninitializedVector(
      hists.cluster_adc_hist.size(), hists.elem_size, &clus_adc_ptr);

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

  auto dataoff = CreateGEMHist(builder, x_strip_off, y_strip_off, x_adc_off,
                               y_adc_off, clus_adc_off, hists.bin_width());

  auto SourceNameOffset = builder.CreateString(SourceName);

  auto msg =
      CreateMonitorMessage(builder, SourceNameOffset, DataField::GEMHist, dataoff.Union());

  FinishMonitorMessageBuffer(builder, msg);

  Buffer<uint8_t> buffer(builder.GetBufferPointer(), builder.GetSize());

  if (producer_callback) {
    producer_callback(buffer);
  }

  return buffer.size;
}
