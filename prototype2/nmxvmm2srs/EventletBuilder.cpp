/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxvmm2srs/EventletBuilder.h>
#include <string.h>
#include <common/Trace.h>

EventletBuilder::EventletBuilder(Time time_intepreter, Geometry geometry_interpreter)
  : time_intepreter_(time_intepreter)
  , geometry_interpreter_(geometry_interpreter)
{}

uint32_t EventletBuilder::process_readout(const NMXVMM2SRSData& data, Clusterer& clusterer) {
  uint16_t fec_id = 1; /**< @todo not hardcode */
  uint16_t chip_id = data.srshdr.dataid & 0xf; /**< @todo may belong elswhere */

  Eventlet eventlet;
  for (unsigned int i = 0; i < data.elems; i++) {
    XTRACE(PROCESS, DEB, "eventlet timestamp: hi 0x%08x, lo: 0x%08x\n", data.srshdr.time, (data.data[i].bcid << 16) +  data.data[i].tdc);
    XTRACE(PROCESS, DEB, "eventlet  chip: %d, strip: %d\n", chip_id, data.data[i].chno);
    eventlet.time = time_intepreter_.timestamp(data.srshdr.time, data.data[i].bcid, data.data[i].tdc);
    eventlet.plane_id = geometry_interpreter_.get_plane_ID(fec_id, chip_id);
    eventlet.strip = geometry_interpreter_.get_strip_ID(fec_id, chip_id, data.data[i].chno);
    eventlet.adc = data.data[i].adc;
    /**< @todo flags? */
    clusterer.insert(eventlet);
  }

  return data.elems;
}