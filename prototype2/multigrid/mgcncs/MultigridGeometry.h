/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Specifying the geometry of a Multi Grid Detector, provides
/// a calculation of global detector pixel id
///
//===----------------------------------------------------------------------===//


#pragma once
#include <cinttypes>
#include <common/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

class MultiGridGeometry {
public:
  /// \brief Create a MG Geometry based on number of columns, grids and
  /// grid wire layout. All are input data
  /// \param panels Number of panels in detector
  /// \param modules Number of modules (columns) per panel
  /// \param grids Number of grids per module
  /// \param xwires Number of wires in the x-direction
  /// \param zwires Number of wires in the z-direction
  MultiGridGeometry(int panels, int modules, int grids, int xwires, int zwires)
      : panls_(panels), mods_(modules), grids_(grids), xwires_(xwires),
        zwires_(zwires) {}

  /// \brief returns the maximum available pixelid for this geometry
  int getmaxpixelid() { return xwires_ * zwires_ * grids_ * mods_ * panls_; }

  /// \brief Return the global detector pixel id from wires and grids
  /// \param panel Panel ID, from module id of readoutsystem
  /// \param gridid Grid ID, calculated from adc values
  /// \param wireid Wire ID , calculated from adc values
  inline int getdetectorpixelid(int panel, int gridid, int wireid) {
    XTRACE(PROCESS, DEB, "panel %d, gridid %d, wireid %d", panel, gridid,
           wireid);

    if ((panel < 1) || (gridid < 1) || (wireid < 1)) {
      XTRACE(PROCESS, WAR,
             "undersize geometry: panel %d, gridid %d, wireid %d", panel,
             gridid, wireid);
      return -1;
    }

    if ((panel > panls_) || (gridid > mods_ * grids_) ||
        (wireid > (mods_ * xwires_ * zwires_))) {
      XTRACE(PROCESS, WAR,
             "oversize geometry: panel %d, gridid %d, wireid %d", panel,
             gridid, wireid);
      return -1;
    }

    int column = mods_ - (wireid - 1) / (zwires_ * xwires_);
    // printf("debug: %d\n", (wireid -1)/(zwires_ * xwires_));
    // printf("column: %d\n", column);
    int gridmin = (column - 1) * grids_ + 1;
    int gridmax = gridmin + grids_ - 1;
    XTRACE(PROCESS, DEB, "grid: %d, min: %d, max: %d", gridid, gridmin,
           gridmax);
    if ((gridid < gridmin) || (gridid > gridmax)) {
      XTRACE(PROCESS, WAR, "geometry mismatch: wire %d, grid %d", wireid,
             gridid);
      return -1;
    }

    int panel_offset = mods_ * xwires_ * (panel - 1);
    int x_offset = mods_ * xwires_ - (wireid - 1) / zwires_;
    XTRACE(PROCESS, DEB, "panel_offset: %d", panel_offset);
    XTRACE(PROCESS, DEB, "x_offset: %d", x_offset);
    int x = panel_offset + x_offset;
    int y = (gridid - 1) % grids_ + 1;
    int z = (wireid - 1) % zwires_ + 1;
    int pixelid = (x - 1) * grids_ * zwires_ + (y - 1) * zwires_ + z;
    XTRACE(PROCESS, DEB, "x: %d, y: %d, z: %d (pixel %d)", x, y, z, pixelid);

    return pixelid;
  }

  /// \brief return logical x-coordinate of a pixel
  int getxcoord(int pixelid) { return (pixelid - 1) / (grids_ * zwires_) + 1; }

  /// \brief return logical x-coordinate of a pixel
  int getycoord(int pixelid) { return ((pixelid - 1) / zwires_) % grids_ + 1; }

  /// \brief return logical x-coordinate of a pixel
  int getzcoord(int pixelid) { return (pixelid - 1) % zwires_ + 1; }

private:
  int panls_;  ///< number of panels per detector
  int mods_;   ///< number of modules per panel
  int grids_;  ///< number of grids per module (grid column)
  int xwires_; ///< number of cells in the x-direction
  int zwires_; ///< number of cells in the z-direction
};
