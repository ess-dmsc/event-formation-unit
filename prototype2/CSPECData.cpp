/** Copyright (C) 2016 European Spallation Source */

#include <CSPECData.h>
#include <iostream>

using namespace std;

struct multi_grid {
  unsigned int header;
  unsigned int data[8];
  unsigned int footer;
} __attribute__((packed));

CSPECData::CSPECData(char *buffer, int size) {
  while (size >= 4 * 10) { // Enough data for processing

    auto mg = (struct multi_grid *)buffer;

    /** Header processing */
    if ((mg->header & 0xc0000000) != 0x40000000) {
      ierror++;
      return;
    }

    if ((mg->header & 0xfff) != 9) {
      ierror++;
      return;
    }
    data.module = (mg->header >> 16) & 0xff;

    /** Footer processing */
    if ((mg->footer & 0xc0000000) != 0xc0000000) {
      ierror++;
      return;
    }

    data.time = mg->footer & 0x3fffffff;

    /** Data processing */
    for (int i = 0; i != 8; ++i) {
      if ((mg->data[i] & 0xc0000000) != 0x00000000) {
        ierror++;
        return;
      }
      data.d[i] = mg->data[i] && 0x3fff;
    }
    dataq.push(data);
    idata++;
    size -= 4 * 10;
  }

  if (size != 0) {
    ifrag++;
  }
}

/** first multi grid data generator - valid headers, all zero data*/
int CSPECData::generate(char *buffer, int size, int elems) {
  int bytes = 0;
  auto mg = (struct multi_grid *)buffer;
  while ((size >= 40) && elems) { // FIXME hardcoded
    mg->header = 0x40000009;
    for (int i = 0; i != 8; ++i) {
      mg->data[i] = 0x00000000;
    }
    mg->footer = 0xc0000000;
    mg++;
    elems--;
    size -= 40;  // FIXME hardcoded
    bytes += 40; // FIXME hardcoded
  }
  return bytes;
}
