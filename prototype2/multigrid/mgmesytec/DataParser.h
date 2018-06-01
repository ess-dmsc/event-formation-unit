/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/FBSerializer.h>
#include <common/DataSave.h>
#include <common/ReadoutSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/MG24Detector.h>

class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  // clang-format off
  // sis3153 and mesytec data types from
  // Struck: mvme-src-0.9.2-281-g1c4c24c.tar
  // Struck: Ethernet UDP Addendum revision 107
  // Mesytec Datasheet: VMMR-8/16 v00.01
  enum DataType {
    mesytecData              = 0x10000000,
    mesytecExtendedTimeStamp = 0x20000000,
    mesytecHeader            = 0x40000000,
    mesytecTimeStamp         = 0xc0000000,
    sisBeginReadout          = 0xbb000000,
    sisEndReadout            = 0xee000000
  };
  // clang-format on

/// @
  MesytecData(uint32_t module, std::string fileprefix = "") {
    mgseq.select_module(module);
    dumptofile = !fileprefix.empty();
    if (dumptofile) {
      mgdata = std::make_shared<DataSave>(fileprefix, 100000000);
      mgdata->tofile("#Time, Bus, Address, ADC\n");
    }
  };

  ~MesytecData(){};

  int getPixel(); // @todo (too) simple implm. but agreed for now
  int getTime();  // @todo (too) simple implm. but agreed for now

  void setWireThreshold(int low, int high) {
     wireThresholdLo = low;
     wireThresholdHi = high;
   };

  void setGridThreshold(int low, int high) {
     gridThresholdLo = low;
     gridThresholdHi = high;
   };
  /** @brief parse a binary payload buffer, return number of data element
   * @todo Uses NMXHists  - refactor and move ?
   */
  error parse(const char *buffer, int size, NMXHists &hists, FBSerializer & fbserializer, ReadoutSerializer &serializer);

  /** @brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t *buffer, int nWords, NMXHists &hists, ReadoutSerializer &serializer);

  // Statistics returned by parse()
  int readouts{0}; /**< number of channels read out */
  int discards{0};
  int triggers{0};
  int events{0};
  int tx_bytes{0};
  int geometry_errors{0};

private:
  int wiremax{-1}; // initial alg.: wire with max adc
  int gridmax{-1}; // initial alg.: grid with max adc
  int time{-1};
  int wireThresholdLo{0};
  int wireThresholdHi{65535};
  int gridThresholdLo{0};
  int gridThresholdHi{65535};
  MG24Detector mgseq;
  ESSGeometry mg{4, 48, 20, 1};

  bool dumptofile{false};
  std::shared_ptr<DataSave>(mgdata);
};
