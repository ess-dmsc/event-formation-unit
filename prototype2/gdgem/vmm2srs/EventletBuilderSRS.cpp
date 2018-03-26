/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <gdgem/vmm2srs/EventletBuilderSRS.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

BuilderSRS::BuilderSRS(SRSTime time_intepreter,
                       SRSMappings geometry_interpreter, std::string dump_dir,
                       bool dump_csv, bool dump_h5)
    : AbstractBuilder(dump_dir, dump_csv, dump_h5), parser_(1125),
      time_intepreter_(time_intepreter),
      geometry_interpreter_(geometry_interpreter) {
#ifdef DUMPTOFILE
  if (dump_csv_) {
    vmmsave->tofile("# fec, chip_id, frame counter, srs timestamp, channel, "
                    "bcid, tdc, adc, overthreshold\n");
  }
#endif
}

AbstractBuilder::ResultStats BuilderSRS::process_buffer(char *buf, size_t size,
                                                        Clusterer &clusterer,
                                                        NMXHists &hists) {
  parser_.receive(buf, size);
  if (!parser_.elems)
    return AbstractBuilder::ResultStats();

  uint16_t fec_id = 1; /**< @todo not hardcode */
  uint16_t chip_id =
      parser_.srshdr.dataid & 0xf; /**< @todo may belong elswhere */
  uint32_t geom_errors{0};

  Eventlet eventlet;
  for (unsigned int i = 0; i < parser_.elems; i++) {
    auto &d = parser_.data[i];
    eventlet.time =
        time_intepreter_.timestamp(parser_.srshdr.time, d.bcid, d.tdc);
    eventlet.plane_id = geometry_interpreter_.get_plane(fec_id, chip_id);
    eventlet.strip = geometry_interpreter_.get_strip(fec_id, chip_id, d.chno);
    eventlet.adc = d.adc;
    eventlet.over_threshold = (d.overThreshold != 0);

    XTRACE(PROCESS, DEB,
           "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x",
           parser_.srshdr.time, d.bcid, d.tdc);
    XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d", chip_id, d.chno);
    XTRACE(PROCESS, DEB,
           "eventlet plane_id: %d, strip: %d, time: %" PRIu64,
           eventlet.plane_id, eventlet.strip, eventlet.time);
    /**< @todo flags? */

    if (eventlet.plane_id == NMX_INVALID_PLANE_ID) {
      geom_errors++;
      XTRACE(PROCESS, DEB, "Bad SRS mapping --  fec: %d, chip: %d", fec_id,
             chip_id);
    } else {
      hists.bin(eventlet);
      clusterer.insert(eventlet);
    }
#ifdef DUMPTOFILE
    if (dump_csv_) {
      vmmsave->tofile("%2d, %2d, %u, %u, %2d, %d, %d, %d, %d\n", fec_id,
                      chip_id, parser_.srshdr.fc, parser_.srshdr.time, d.chno,
                      d.bcid, d.tdc, d.adc, d.overThreshold);
    }
#endif
      //printf("readout: time: %llu, plane: %d, strip: %d, adc: %d\n", eventlet.time, eventlet.plane_id, eventlet.strip, eventlet.adc);
  }

  return AbstractBuilder::ResultStats(parser_.elems, parser_.error,
                                      geom_errors);
}
