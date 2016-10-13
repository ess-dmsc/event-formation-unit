/** Copyright (C) 2016 European Spallation Source */

#include <CSPECData.h>
#include <iostream>

using namespace std;

struct multi_grid {
  unsigned int header;
  unsigned int data[8];
  unsigned int footer;
} __attribute__((packed));

int CSPECData::receive(const char *buffer, int size) {
  int elems = 0;
  int error = 0;
  auto mgp = (struct multi_grid *)buffer;

  while (size >= datasize) { // Enough data for processing

    /** Header processing */
    if ((mgp->header & header_mask) != header_id) {
      error = 1;
      break;
    }

    if ((mgp->header & 0xfff) != nwords) {
      error = 1;
      break;
    }
    data[elems].module = (mgp->header >> 16) & 0xff;
    // printf("data[%d].module == %d (header %08x)\n", elems,
    // data[elems].module, mgp->header);

    /** Footer processing */
    if ((mgp->footer & header_mask) != footer_id) {
      error = 1;
      break;
    }
    data[elems].time = mgp->footer & 0x3fffffff;

    /** Data processing */
    for (int i = 0; i != 8; ++i) {
      if ((mgp->data[i] & header_mask) != data_id) {
        error = 1;
        break;
      }
      data[elems].d[i] = mgp->data[i] && 0x3fff;
    }

    // At this point we have a full dataset
    elems++;
    size -= datasize;
    mgp++;
  }

  if (error) {
    ierror++;
    return elems;
  }

  idata += elems;

  if (size != 0) {
    ifrag++;
  }
  return elems;
}

/** first multi grid data generator - valid headers, all zero data*/
int CSPECData::generate(char *buffer, int size, int elems) {
  int bytes = 0;
  auto mg = (struct multi_grid *)buffer;
  while ((size >= CSPECData::datasize) && elems) {
    mg->header = header_id + nwords;
    for (int i = 0; i != 8; ++i) {
      mg->data[i] = data_id;
    }
    mg->footer = footer_id;
    mg++;
    elems--;
    size -= CSPECData::datasize;
    bytes += CSPECData::datasize;
  }
  return bytes;
}
