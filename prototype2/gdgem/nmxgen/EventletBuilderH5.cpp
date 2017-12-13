/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <cstring>
#include <gdgem/nmxgen/EventletBuilderH5.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

BuilderH5::BuilderH5(std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  data.resize(4);
#ifdef DUMPTOFILE
  if (dump_csv_)
    vmmsave->tofile("# time, plane, strip, adc, overthreshold\n");
#endif
}

AbstractBuilder::ResultStats BuilderH5::process_buffer(char *buf, size_t size,
                                                       Clusterer &clusterer,
                                                       NMXHists &hists) {
  size_t count = std::min(size / psize, size_t(9000 / psize));
  for (size_t i = 0; i < count; ++i) {
    memcpy(data.data(), buf, psize);
    auto eventlet = make_eventlet();
    hists.bin(eventlet);
    clusterer.insert(eventlet);
    buf += psize;
  }
  return AbstractBuilder::ResultStats(count, 0, 0);
}

Eventlet BuilderH5::make_eventlet() {
  Eventlet ret;
  ret.time = (uint64_t(data[0]) << 32) | uint64_t(data[1]);
  ret.plane_id = data[2] >> 16;
  ret.strip = data[2] & 0xFFFF;
  ret.flag = (data[3] >> 16) & 0x1;
  ret.over_threshold = (data[3] >> 17) & 0x1;
  ret.adc = data[3] & 0xFFFF;

  XTRACE(PROCESS, DEB, "Made eventlet: %s\n", ret.debug().c_str());

#ifdef DUMPTOFILE
  if (dump_csv_)
    vmmsave->tofile("%" PRIu64 ", %u, %u, %u, %u\n", ret.time, ret.plane_id,
                    ret.strip, ret.adc, ret.over_threshold);

  if (dump_h5_) {
    //    srstime_.write(data[0], {event_num_});
    //    bc_tdc_adc_.write(static_cast<uint16_t>(data[1] >> 16), {event_num_,
    //    0}); bc_tdc_adc_.write(static_cast<uint16_t>(data[1] & 0xFF),
    //    {event_num_, 1}); bc_tdc_adc_.write(ret.adc, {event_num_, 2});
    //    fec_chip_chan_thresh_.write(uint8_t(0), {event_num_, 0});
    //    fec_chip_chan_thresh_.write(ret.plane_id, {event_num_, 1});
    //    fec_chip_chan_thresh_.write(static_cast<uint8_t>(ret.strip),
    //    {event_num_, 2}); fec_chip_chan_thresh_.write(uint8_t(0), {event_num_,
    //    3});
    event_num_++;
  }
#endif

  return ret;
}
