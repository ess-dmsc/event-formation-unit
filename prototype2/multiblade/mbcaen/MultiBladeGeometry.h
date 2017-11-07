/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multiblade geometry
 */

#pragma once
#include <cstdio>

class MultiBladeGeometry {
  public:

  int digid_to_index(int digid) {
    switch (digid) {
      case 137:
        return 0;
      break;
      case 143:
        return 1;
      break;
      case 142:
        return 2;
      break;
      case 31:
        return 3;
      break;
      case 34:
        return 4;
      break;
      case 33:
        return 5;
      break;
      default:
      return -1;
    }
  }

  int pixelid(int digid, int stripid, int wireid) {
    const int ns = 32;
    const int nw = 32;
    const int nsw = ns * nw;

    if ((stripid < 1) || (stripid > ns) || (wireid < 1) || (wireid > nw)) {
      return 0;
    }

    int digidx = digid_to_index(digid);
    if (digidx < 0) {
      return 0;
    }

    //printf("digidx: %d, (wireid -1): %d, (33 - stripid): %d\n", digidx, wireid-1, 33-stripid);

    return  digidx * nsw + (wireid - 1) * ns + (33 - stripid);
  }
};
