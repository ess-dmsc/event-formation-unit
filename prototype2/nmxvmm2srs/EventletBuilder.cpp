/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <nmxvmm2srs/EventletBuilder.h>
#include <string.h>

EventletBuilder::EventletBuilder(Time time_intepreter,
                                 Geometry geometry_interpreter)
    : time_intepreter_(time_intepreter),
      geometry_interpreter_(geometry_interpreter) {}

uint32_t EventletBuilder::process_readout(NMXVMM2SRSData &data, Clusterer &clusterer) {
  uint16_t fec_id = 1;                         /**< @todo not hardcode */
  uint16_t chip_id = data.srshdr.dataid & 0xf; /**< @todo may belong elswhere */

  Eventlet eventlet;
  for (unsigned int i = 0; i < data.elems; i++) {
    auto & d = data.data[i];
    XTRACE(PROCESS, DEB, "eventlet timestamp: hi 0x%08x, lo: 0x%08x\n",
           data.srshdr.time, (d.bcid << 16) + d.tdc);
    XTRACE(PROCESS, DEB, "eventlet  chip: %d, strip: %d\n", chip_id, d.chno);
    eventlet.time = time_intepreter_.timestamp(data.srshdr.time, d.bcid, d.tdc);
    eventlet.plane_id = geometry_interpreter_.get_plane(fec_id, chip_id);
    eventlet.strip = geometry_interpreter_.get_strip(fec_id, chip_id, d.chno);
    eventlet.adc = d.adc;
    /**< @todo flags? */

    assert(eventlet.plane_id == 0 || eventlet.plane_id == 1);
    assert(eventlet.strip <= 1500);
    data.xyhist[eventlet.plane_id][eventlet.strip]++;
    clusterer.insert(eventlet);
  }

  return data.elems;
}
