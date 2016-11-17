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
  /** @brief Create a MG Geometry based on number of columns, grids and
   * grid wire layout.
   * @param columns Number of columns
   * @param grids Number of grids in a column
   * @param xwires Number of wires in the x-direction
   * @param zwires Number of wires in the z-direction
   */
  MultiGridGeometry(int columns, int grids, int xwires, int zwires)
      : cols_(columns), grids_(grids), xwires_(xwires), zwires_(zwires) {}

  /** @brief returns the maximum available pixelid for this geometry
   */
  int getmaxdetectorid() { return xwires_ * zwires_ * grids_ * cols_; }

  /** @brief Return the global detector pixel id from wires and grids
   *  @param column Column ID, from module id of readoutsystem
   *  @param gridid Grid ID, calculated from adc values
   *  @param wireid Wire ID , calculated from adc values
   */
  inline int getdetectorpixelid(int column, int gridid, int wireid) {
    if ((column > cols_) || (gridid > grids_) || (wireid > xwires_ * zwires_)) {
      return -1;
    }
    if ((column < 1) || (gridid < 1) || (wireid < 1)) {
      return -1;
    }

    auto ncxg = cols_ * xwires_ * grids_;
    auto ncx = cols_ * xwires_;
    auto z = (wireid - 1) % zwires_ + 1;
    auto x = (column - 1) * xwires_ + (wireid - 1) / zwires_ + 1;
    // cout << "x: " << x << ", z: " << z << endl;
    return (z - 1) * (ncxg) + (gridid - 1) * ncx + x;
  }

private:
  int cols_;
  int grids_;
  int xwires_;
  int zwires_;
};
