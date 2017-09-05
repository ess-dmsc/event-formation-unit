/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <fcntl.h>
#include <gdgem/vmm2srs/EventletBuilder.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

EventletBuilder::EventletBuilder(SRSTime time_intepreter,
                                 SRSMappings geometry_interpreter)
    : time_intepreter_(time_intepreter),
      geometry_interpreter_(geometry_interpreter) {
#ifdef DUMPTOFILE
  // std::string fileName = "dumpfile_"
  fd = open("dumpfile.txt", O_RDWR | O_CREAT, S_IRWXU);
  assert(fd >= 0);
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char s[128];
  strftime(s, sizeof(s), "%c", tm);
  dprintf(fd, "%s\n", s);
  dprintf(fd, "# fec, chip_id, srs timestamp, channel, bcid, tdc, adc\n");
#endif
}

uint32_t EventletBuilder::process_readout(NMXVMM2SRSData &data,
                                          Clusterer &clusterer) {
  uint16_t fec_id = 1;                         /**< @todo not hardcode */
  uint16_t chip_id = data.srshdr.dataid & 0xf; /**< @todo may belong elswhere */

  Eventlet eventlet;
  for (unsigned int i = 0; i < data.elems; i++) {
    auto &d = data.data[i];
    XTRACE(PROCESS, DEB, "eventlet timestamp: hi 0x%08x, lo: 0x%08x\n",
           data.srshdr.time, (d.bcid << 16) + d.tdc);
    XTRACE(PROCESS, DEB, "eventlet  chip: %d, channel: %d\n", chip_id, d.chno);
    eventlet.time = time_intepreter_.timestamp(data.srshdr.time, d.bcid, d.tdc);
    eventlet.plane_id = geometry_interpreter_.get_plane(fec_id, chip_id);
    eventlet.strip = geometry_interpreter_.get_strip(fec_id, chip_id, d.chno);
    eventlet.adc = d.adc;
    XTRACE(PROCESS, DEB, "eventlet  plane_id: %d, strip: %d\n",
           eventlet.plane_id, eventlet.strip);
/**< @todo flags? */

#ifdef DUMPTOFILE
    dprintf(fd, "%2d, %2d, %u, %2d, %d, %d, %d\n", 1, chip_id, data.srshdr.time,
            d.chno, d.bcid, d.tdc, d.adc);
#endif

    assert(eventlet.plane_id == 0 || eventlet.plane_id == 1);
    assert(eventlet.strip <= 1500);
    data.xyhist[eventlet.plane_id][eventlet.strip]++;
    data.xyhist_elems++;
    clusterer.insert(eventlet);
  }

  return data.elems;
}
