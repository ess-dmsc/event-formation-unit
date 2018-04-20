/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <multigrid/mgcncs/DataParser.h>

struct multi_grid {
  uint32_t header;
  uint32_t data[8];
  uint32_t footer;
} __attribute__((packed));

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

/** @todo no error checking, assumes valid data and valid buffer */
int CSPECData::createevent(const MultiGridData &data, uint32_t *time,
                           uint32_t *pixel) {
  auto panel = data.module;
  auto grid = chanconv->getgridid(data.d[6]);
  auto wire = chanconv->getwireid(data.d[2]);

  XTRACE(PROCESS, DEB, "panel %u, grid %d, wire %d\n", panel, grid, wire);

  /** @todo eventually get rid of this, but electronics is wrongly wired
   * on the prototype detector currently being tested
   */

  if (wire & 1) {
    wire++;
  } else {
    wire--;
  }

#if 0
  if (grid <= 48) {
    grid += 48;
  } else {
    grid -= 49;
  }
#endif

  auto pixid = multigridgeom->getdetectorpixelid(panel, grid, wire);
  if (pixid < 1) {
    XTRACE(PROCESS, WAR, "panel %u, grid %d, wire %d, pixel %d\n", panel, grid,
           wire, pixid);
    return -1;
  }

  XTRACE(PROCESS, INF, "panel %u, grid %d, wire %d, pixel %d\n", panel, grid,
         wire, pixid);

  static_assert(sizeof(data.time) == 4, "time should be 32 bit");
  static_assert(sizeof(pixid) == 4, "pixelid should be 32 bit");

  *time = data.time;
  *pixel = pixid;
  return 0;
}

int CSPECData::receive(const char *buffer, int size) {
  elems = 0;
  error = 0;
  enum State { hdr = 1, dat, ftr };
  unsigned int datctr = 0;
  unsigned int channel = 0;
  uint32_t *datap = (uint32_t *)buffer;
  int state = State::hdr;
  int oldsize = size;

  while (size >= 4) {
    // XTRACE(PROCESS, DEB, "elems: %d, size: %d, datap: %p\n", elems, size,
    // datap);
    switch (state) {
    // Parse Header
    case State::hdr:
      if (((*datap & header_mask) != header_id) ||
          ((*datap & 0xfff) != nwords)) {
        XTRACE(PROCESS, INF, "State::hdr - header error\n");
        break;
      }
      // XTRACE(PROCESS, DEB, "State::hdr valid data, next state State:dat\n");
      data[elems].module = (*datap >> 16) & 0xff;
      datctr = 0;
      state = State::dat;
      break;

    // Parse Data
    case State::dat:
      if ((*datap & header_mask) != data_id) {
        XTRACE(PROCESS, INF, "State::dat - header error\n");
        state = State::hdr;
        break;
      }
      // XTRACE(PROCESS, DEB, "State::dat valid data (%d), next state
      // State:dat\n",
      //       datctr);
      channel = ((*datap) >> 16) & 0xff;
      // assert(channel == datctr); // asserts for 2016_08_16_0921_sample_ in
      // late 9000s
      if (channel != datctr) {
        XTRACE(PROCESS, WAR, "State::dat - data order mismatch\n");
        state = State::hdr;
        break;
      }

      data[elems].d[datctr] = (*datap) & 0x3fff;
      XTRACE(PROCESS, DEB, "data[%u].d[%u]: %u\n", elems, datctr,
             data[elems].d[datctr]);
      datctr++;
      if (datctr == 8) {
        // XTRACE(PROCESS, DEB, "State:dat all data, next state State:ftr\n");
        state = State::ftr;
      }
      break;

    // Parse Footer
    case State::ftr:
      if ((*datap & header_mask) != footer_id) {
        XTRACE(PROCESS, WAR, "State::ftr - header error\n");
        state = State::hdr;
        break;
      }
      // XTRACE(PROCESS, DEB,
      //       "State::ftr valid data, next state State:hdr, events %u\n",
      //       elems);
      data[elems].time = (*datap) & 0x3fffffff;
      XTRACE(PROCESS, DEB, "time: %u\n", data[elems].time);
      elems++;
      state = State::hdr;

#ifdef DUMPTOFILE
      auto dp = data[elems];
      mgdata.tofile("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n", dp.module, dp.time,
                    dp.d[0], dp.d[1], dp.d[2], dp.d[3], dp.d[4], dp.d[5],
                    dp.d[6], dp.d[7]);
#endif

      break;
    }
    size -= 4; // Parse 32 bit at a time
    datap++;
  }
  error = oldsize - (elems * datasize);

  return elems;
}

int CSPECData::input_filter() {
  int discarded = 0;
  if (elems == 0)
    return discarded;

  for (unsigned int i = 0; i < elems; i++) {
    data[i].valid = 0;
    if ((data[i].d[0] < wire_thresh) || (data[i].d[4] < grid_thresh)) {
      XTRACE(PROCESS, INF, "data 0 or 4 failed, thresholds: %u, %u\n",
             wire_thresh, grid_thresh);
      discarded++; // due to low signal
      continue;
    }
    data[i].valid = 1;

    if (data[i].d[1] >= wire_thresh) {
      XTRACE(PROCESS, INF, "data 1 or 5 failed, thresholds: %u, %u\n",
             wire_thresh, grid_thresh);
      discarded++;       // due to duplicate neutron event
      data[i].valid = 0; // invalidate
      continue;
    }
  }
  return discarded;
}

/** First multi grid data generator - valid headers, custom
 * wire and grid adc values. Only used in google test - can
 * be excluded from coverage
 */
int CSPECData::generate(char *buffer, int size, int elems,
                        unsigned int wire_adc, unsigned int grid_adc) {
  int bytes = 0;
  int events = 0;
  auto mg = (struct multi_grid *)buffer;
  while ((size >= CSPECData::datasize) && elems) {
    events++;
    mg->header = header_id + (1 << 16) + nwords;
    for (int i = 0; i != 8; ++i) {
      mg->data[i] = data_id + (i << 16);
    }
    mg->data[0] += wire_adc;
    mg->data[4] += grid_adc;

    mg->footer = footer_id + events;
    mg++;
    elems--;
    size -= CSPECData::datasize;
    bytes += CSPECData::datasize;
  }
  return bytes;
}
