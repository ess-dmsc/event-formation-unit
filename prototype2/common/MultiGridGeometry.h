/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Specifying the geometry of a Multi Grid Detector, provides
 * a calculation of global detector pixel id
 */

#pragma once
#include <cinttypes>

using namespace std;

class MultiGridGeometry {
public:
  MultiGridGeometry(int columns, int grids, int xwires, int zwires)
    : cols_(columns), grids_(grids), xwires_(xwires), zwires_(zwires) {}

int getmaxdetectorid() {return xwires_ * zwires_ * grids_ * cols_;}

inline int getdetectorid(int column, int gridid, int wireid) {
   if ((column > cols_) || (gridid > grids_) || (wireid > xwires_ * zwires_)) {
     return -1;
   }
   if ((column < 1) || (gridid < 1) || (wireid < 1)) {
     return -1;
   }

   auto nxy = cols_ * xwires_ * grids_;
   auto nx = cols_ * xwires_;
   auto z = (wireid - 1) % zwires_ + 1;
   auto x = (column - 1) * xwires_ + (wireid -1) / zwires_ + 1;
   //cout << "x: " << x << ", z: " << z << endl;
   return (z - 1) * (nxy) + (gridid - 1)* nx + x;
}

private:
  int cols_;
  int grids_;
  int xwires_;
  int zwires_;
};
