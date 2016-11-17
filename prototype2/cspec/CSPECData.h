/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once

#include <common/MultiGridGeometry.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CSPECEvent.h>

class CSPECData {
public:
  // See "multi grid detector data processing.docx" from DMG's
  // Event Formation Files
  unsigned int wire_thresh{230}; // From Anton 4 oct 2016
  unsigned int grid_thresh{170}; //  ...

  const int datasize = 40; /**< size (bytes) of a data readout */
  // clang-format off
  const unsigned int header_mask = 0xc0000000;
  const unsigned int header_id =   0x40000000;
  const unsigned int data_id =     0x00000000;
  const unsigned int footer_id =   0xc0000000;
  const unsigned int nwords = 9;
  // clang-format on

  struct MultiGridData {
    unsigned int module;
    unsigned int d[8];
    unsigned int time;
    unsigned int valid;
  };

  /** Let user specify calibration parameters */
  CSPECData(CSPECChanConv *calibration, MultiGridGeometry *geometry)
      : chanconv(calibration), multigridgeom(geometry){};

  CSPECData(unsigned int wthresh, unsigned int gthresh,
            CSPECChanConv *calibration, MultiGridGeometry *geometry)
      : wire_thresh(wthresh), grid_thresh(gthresh), chanconv(calibration),
        multigridgeom(geometry){};

  CSPECData(){}; // Discouraged, but used in cspecgen

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);

  /** Discard data below threshold, double events, etc., return number
  *  of discarded samples */
  int input_filter();

  /** Generate simulated data, place in user specified buffer */
  int generate(char *buffer, int size, int elems, unsigned int wire_adc,
               unsigned int grid_adc);

  /** @brief serialize event to buffer
   *  @param data Multi grid data from event readout system
   *  @param buffer User specified buffer (must be large enough to hold event
   */
  void createevent(const MultiGridData &data, char *buffer);

  // This data is overwritten on receive()
  struct MultiGridData data[250];
  unsigned int elems{0};
  unsigned int error{0};

  CSPECChanConv *chanconv{nullptr};
  MultiGridGeometry *multigridgeom{nullptr};
};
