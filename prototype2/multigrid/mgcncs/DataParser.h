/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to receive and generate CSPEC detector readout
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DataSave.h>
#include <multigrid/mgcncs/ChanConv.h>
#include <multigrid/mgcncs/MultigridGeometry.h>
#include <memory>

class CSPECData {
public:
  /// See "multi grid detector data processing.docx" from DMG's
  /// Event Formation Files
  unsigned int wire_thresh{230}; /// From Anton 4 oct 2016
  unsigned int grid_thresh{170}; ///         -||-

  const int datasize = 40; ///< size (bytes) of a data readout
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

  /// Let user specify calibration parameters
  CSPECData(unsigned int maxevents,
            CSPECChanConv *calibration,
            MultiGridGeometry *geometry,
            std::string fileprefix = "")
      : datalen(maxevents), chanconv(calibration), multigridgeom(geometry) {

    data = new struct MultiGridData[maxevents];

    dumptofile = !fileprefix.empty();
    if (dumptofile) {
      mgdata = std::make_shared<DataSave>(fileprefix, 100000000);
      mgdata->tofile("#module, time, d0, d1, d2, d3, d4, d5, d6, d7\n");
    }
  };

  CSPECData(unsigned int maxevents, unsigned int wthresh, unsigned int gthresh,
            CSPECChanConv *calibration, MultiGridGeometry *geometry)
      : wire_thresh(wthresh), grid_thresh(gthresh), datalen(maxevents),
        chanconv(calibration), multigridgeom(geometry) {
    data = new struct MultiGridData[maxevents];
  };

  CSPECData(){}; /// Discouraged, but used in cspecgen

  ~CSPECData() {
    if (data != nullptr) {
      delete[] data;
    }
  }

  /// \brief parse a binary payload buffer, return number of data elements
  int receive(const char *buffer, int size);

  /// Discard data below threshold, double events, etc., return number
  /// of discarded samples
  int input_filter();

  /// Generate simulated data, place in user specified buffer
  int generate(char *buffer, int size, int elems, unsigned int wire_adc,
               unsigned int grid_adc);

  /// \brief serialize event to buffer
  /// \param data Multi grid data from event readout system
  /// \param buffer User specified buffer (must be large enough to hold event
  /// \todo document return value
  int createevent(const MultiGridData &data, uint32_t *time, uint32_t *pixel);

  /// \note This data is overwritten on receive()
  struct MultiGridData *data{nullptr};
  unsigned datalen{0};
  unsigned int elems{0};
  unsigned int error{0};

  CSPECChanConv *chanconv{nullptr};
  MultiGridGeometry *multigridgeom{nullptr};

private:
  bool dumptofile{false};
  std::shared_ptr<DataSave>(mgdata);
};
