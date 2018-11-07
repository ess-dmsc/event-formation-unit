
#pragma once

#include <logical_geometry/ESSGeometry.h>

uint32_t TestImage2D(uint64_t time, ESSGeometry * essgeom) {
  auto width = essgeom->nx();
  auto height = essgeom->ny();

  if (time % 10000 == 0) {
    return essgeom->pixel2D(width - 1, height - 1);
  }

  uint32_t x, y;
  auto t = time % 3;

  x = 0;
  y = 0;
  if (t == 0) { // top line
     auto nt = (time/3) % (width/2);
     x = nt + width/4;
     y = height/4;
  } else if (t == 1) { // vertical line
    auto nt = ((time - 1)/3) % (height/2);
    x = width/4;
    y = nt + height/4;
  }  else if (t == 2) { // middle line
    auto nt = ((time - 2)/3) % (width/4);
    x = nt + width/4;
    y = height/2;
  }

  return essgeom->pixel2D(x, y);
}
