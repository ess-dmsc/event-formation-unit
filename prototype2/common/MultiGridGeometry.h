/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Specifying the geometry of a Multi Grid Detector, provides
 * a calculation of global detector pixel id
 */

#pragma once
#include <cinttypes>
#include <common/Trace.h>

#define UNUSED __attribute__((unused))

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace std;

class MultiGridGeometry {
public:
  /** @brief Create a MG Geometry based on number of columns, grids and
   * grid wire layout. All are input data
   * @param panels Number of panels in detector
   * @param modules Number of modules (columns) per panel
   * @param grids Number of grids per module
   * @param xwires Number of wires in the x-direction
   * @param zwires Number of wires in the z-direction
   * @param panel_off offset from readout (should be 0 in real detector)
   * @param swap_wires swap 1<->2, 3<->4, etc. (should be 0 in real detector)
   */
  MultiGridGeometry(int panels, int modules, int grids, int xwires,
                    int zwires, int panel_off, int swap_wires)
      : panls_(panels), mods_(modules), grids_(grids), xwires_(xwires),
        zwires_(zwires), poff_(panel_off), swapw_(swap_wires) {}

  /** @brief returns the maximum available pixelid for this geometry
   */
  int getmaxpixelid() { return xwires_ * zwires_ * grids_ * mods_ * panls_; }

  /** @brief Return the global detector pixel id from wires and grids
   *  @param panel Panel ID, from module id of readoutsystem
   *  @param gridid Grid ID, calculated from adc values
   *  @param wireid Wire ID , calculated from adc values
   */
  inline int getdetectorpixelid(int panel, int gridid, int wireid) {
    if (((panel + poff_) < 1) || (gridid < 1) || (wireid < 1)) {
      XTRACE(PROCESS, WAR, "undersize geometry: panel %d, (off %d) gridid %d, wireid %d\n",
             panel, poff_, gridid, wireid);
      return -1;
    }

    if ((panel + poff_ > panls_) || (gridid > grids_) || (wireid > (mods_ * xwires_ * zwires_) )) {
      XTRACE(PROCESS, WAR, "oversize geometry: panel %d, (off %d) gridid %d, wireid %d\n",
             panel, poff_, gridid, wireid);
      return -1;
    }

    /** @todo eventurlly get rid of this, but electronics is wrongly wired
     * on the prototype detector currently being tested
     */
    if (swapw_) {
       if (wireid & 1) {
         wireid++;
       } else {
         wireid--;
       }
    }

    int x = (wireid -1) / zwires_;

    int y = gridid / mods_ + 1;
    int z = (wireid - 1) % zwires_ + 1;

    XTRACE(PROCESS, DEB, "(w, g) %d, %d, (x,y,z) %d %d %d\n", wireid, gridid, x, y, z);
    int nyz = grids_ * zwires_;
    int nxyz = nyz * xwires_ * mods_;
    return (y - 1) * zwires_ + z + x * nyz + nxyz * (panel + poff_ - 1);
  }

private:
  int panls_;  /**< number of panels per detector */
  int mods_;   /**< number of modules per panel */
  int grids_;  /**< number of grids per module (grid column) */
  int xwires_; /**< number of cells in the x-direction */
  int zwires_; /**< number of cells in the z-direction */
  int poff_;   /**< panel offset - temporary hack, shuld be 0 in production */
  int swapw_;  /**< If nonzero: swap wireids 1-2, 3-4, 5-6, ... */
};
