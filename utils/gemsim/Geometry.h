/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple Geometry Class
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cmath>
#include <cassert>

class Geometry {
public:
  // width and height in mm
  Geometry(int width, int height, int nx, int ny)
      : width_(width), height_(height), nx_(nx), ny_(ny) {
    dx = width_*1.0/nx_;
    dy = height_*1.0/ny_;
    srand(42);
    printf("Geometry: strip spacing x: %f mm, y: %f mm\n", dx, dy);
  }

  int getxStrip(double x) {
    assert(std::abs(x) <= nx_*1.0/2);
    int res =  std::rint(x/dx) + 640;
    if (res == 0) { // fixme in google test
      res = 1;
    }
    assert((res >= 1) && (res <= 1280));
    return res;
  }

  int getyStrip(double y) {
    assert(std::abs(y) <= ny_*1.0/2);
    int res =  std::rint(y/dy) + 640;
    if (res == 0) { // fixme in google test
      res = 1;
    }
    assert((res >= 1) && (res <= 1280));
    return res;
  }

  double getRandomX() {
      return (1.0 * width_ * (rand() % nx_) + 1)/nx_ - width_/2.0;
  }

  double getRandomY() {
      return (1.0 * height_ * (rand() % ny_) + 1)/ny_ - height_/2.0;
  }

private:
  int width_{600};
  int height_{600};
  int nx_{1280};
  int ny_{1280};
  double dx{600/1280}; //(0,0) is center
  double dy{600/1280};
};
